
#include "gsm/decoder.h"

#include <iostream>

#include "gsm/state_machine/synchronize.h"
#include "gsm/constants.h"
#include "helper_functions.h"

uint8_t
GSM::Decoder::sync_parity_polynomial_[kSyncParitySize + 1] = {
  1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1
};

uint8_t
GSM::Decoder::sync_parity_remainder_[kSyncParitySize] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};


uint8_t
GSM::Decoder::normal_parity_polynomial_[kNormalParitySize + 1] = {
  1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 1, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0,
  1
};

uint8_t
GSM::Decoder::normal_parity_remainder_[kNormalParitySize] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1
};

GSM::Decoder::Decoder(GSM::StateMachine::Synchronize *ssm) {
  // Pass along a pointer such that the parser can set sync variables
  // (t1, t2, t3 etc.)
  ssm_ = ssm;
  parser_ = new GSM::Parser(ssm);

  uint32_t k, j, B;
  for (k = 0; k < kNormalConvSize; k++) {
    B = k % 4;
    j = 2 * ((49 * k) % 57) + ((k % 8) / 4);
    interleave_table_[k] = B * 114 + j;
  }
}

bool GSM::Decoder::Decode(uint8_t *encoded_burst,
                          std::string burst_type) {
  uint8_t *buffer_ptr, *decoded_buffer_ptr,
      *parity_polynomial, *parity_remainder;

  uint32_t buffer_length, max_error, data_size;
  uint32_t parity_size;
  uint8_t errors;

  // A synchronization burst is not interleaved nor is it as long as a
  // normal burst.
  if (!burst_type.compare("Synchronization")) {
    uint8_t data_buffer[kSyncConvSize], decoded_data_buffer[kSyncParityOutputSize];
    memcpy(data_buffer, encoded_burst + kTailBits, kSyncDataLen);
    memcpy(data_buffer + kSyncDataLen, encoded_burst + kTailBits + kSyncDataLen + kNumSyncBits, kSyncDataLen);
    buffer_ptr = data_buffer;
    decoded_buffer_ptr = decoded_data_buffer;
    buffer_length = kSyncConvSize;
    data_size = kSyncDataBlockSize;
    parity_size = kSyncParitySize;

    parity_polynomial = sync_parity_polynomial_;
    parity_remainder = sync_parity_remainder_;
  } else {
    uint8_t *burst_buffer = ssm_->burst_counter_->get_buffer();
    uint32_t num_stored_bursts = ssm_->burst_counter_->num_stored_bursts();

    if (num_stored_bursts == kNormalBlocks) {
      uint8_t interleaved_data[kNormalBlocks * kNormalBlockSize];
      uint8_t data_buffer[kNormalConvSize];
      uint8_t decoded_data_buffer[kNormalParityOutputSize];

      uint32_t i, j;
      for (i = 0; i < kNormalBlocks; i++) {
        for (j = 0; j < kPayloadBits; j++) {
          interleaved_data[i * kNormalBlockSize + j] = burst_buffer[i * kBurstSize + j + 3];
          interleaved_data[i * kNormalBlockSize + j + kPayloadBits] = burst_buffer[i * kBurstSize + j + 88];
        }
      }
      ssm_->burst_counter_->set_num_stored_bursts(0);

      decoded_buffer_ptr = decoded_data_buffer;
      buffer_length = kNormalConvSize;
      data_size = kNormalDataBlockSize;
      parity_size = kNormalParitySize;

      parity_polynomial = normal_parity_polynomial_;
      parity_remainder = normal_parity_remainder_;

      for (i = 0; i < buffer_length; i++) {
        data_buffer[i] = interleaved_data[interleave_table_[i]];
      }

      buffer_ptr = data_buffer;
    } else {
      // Causes segfault some times
      memcpy(burst_buffer + num_stored_bursts * kBurstSize, encoded_burst, kBurstSize);

      ssm_->burst_counter_->set_num_stored_bursts(num_stored_bursts + 1);

      return 0;
    }
  }

  max_error = 2 * buffer_length + 1;

  // Viterbi decode
  errors = Deconvolve(&buffer_ptr[0], &decoded_buffer_ptr[0], buffer_length, max_error);
  if (errors) {
    // Handle errors
  }

  // for (int i = 0; i < data_size; i++) {
  //   printf("%d", decoded_buffer_ptr[i]);
  // }
  // printf("\n");

  // check parity
  if (CheckParity(&decoded_buffer_ptr[0],
                  parity_polynomial, parity_remainder,
                  data_size, parity_size)) {
    printf("Decode failed!\n");
    return 0;
  }

  printf("Decode was a success!\n");

  parser_->Parse(&decoded_buffer_ptr[0], data_size, burst_type);

  return 1;
}

int32_t GSM::Decoder::Deconvolve(uint8_t *data,
                                 uint8_t *output,
                                 uint32_t conv_output_size,
                                 uint32_t max_error) {
  uint32_t i, t;
  uint32_t rdata, state, nstate, b, o, distance, accumulated_error,
  min_state, min_error, cur_state;

  uint32_t ae[kPolynomialK];
  uint32_t nae[kPolynomialK]; // next accumulated error
  uint32_t state_history[kPolynomialK][conv_output_size + 1];

  // initialize accumulated error, assume starting state is 0
  for (i = 0; i < (kPolynomialK); i++) {
    ae[i] = nae[i] = max_error;
  }
  ae[0] = 0;

  // build trellis
  for (t = 0; t < conv_output_size; t++) {

    // get received data symbol
    rdata = (data[2 * t] << 1) | data[2 * t + 1];

    // for each state
    for (state = 0; state < (kPolynomialK); state++) {

      // make sure this state is possible
      if (ae[state] >= max_error) {
        continue;
      }

      // find all states we lead to
      for (b = 0; b < 2; b++) {

        // get next state given input bit b
        nstate = kNextState[state][b];

        // find output for this transition
        o = kEncode[state][b];

        // calculate distance from received data
        distance = HammingDistance(rdata ^ o);

        // choose surviving path
        accumulated_error = ae[state] + distance;
        if (accumulated_error < nae[nstate]) {

          // save error for surviving state
          nae[nstate] = accumulated_error;

          // update state history
          state_history[nstate][t + 1] = state;
        }
      }
    }

    // get accumulated error ready for next time slice
    for (i = 0; i < kPolynomialK; i++) {
      ae[i] = nae[i];
      nae[i] = max_error;
    }
  }

  // the final state is the state with the fewest errors
  min_state = (uint32_t) - 1;
  min_error = max_error;
  for (i = 0; i < kPolynomialK; i++) {
    if (ae[i] < min_error) {
      min_state = i;
      min_error = ae[i];
    }
  }

  // trace the path
  cur_state = min_state;
  for (t = conv_output_size; t >= 1; t--) {
    min_state = cur_state;
    cur_state = state_history[cur_state][t]; // get previous
    output[t - 1] = kPrevNextState[cur_state][min_state];
  }

  // return the number of errors detected (hard-decision)
  return min_error;
}

int32_t GSM::Decoder::CheckParity(uint8_t *d,
                                  uint8_t *parity_polynomial_,
                                  uint8_t *parity_remainder_,
                                  uint32_t data_block_size,
                                  uint32_t parity_size) {
  uint32_t i;
  uint8_t buf[data_block_size + parity_size], *q;

  memcpy(buf, d, data_block_size + parity_size);

  for (q = buf; q < buf + data_block_size; q++) {
    if (*q) {
      for (i = 0; i < parity_size + 1; i++) {
        q[i] ^= parity_polynomial_[i];
      }
    }
  }

  return memcmp(buf + data_block_size, parity_remainder_, parity_size);
}
