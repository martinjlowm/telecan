/* -*- c++ -*- */
/*
 * Copyright (c) 2015, Martin Jesper Low Madsen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     *  Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *     *  Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include "helper_functions.h"
#include "gsm/sequence_estimator.h"
#include "gsm/decoder.h"
#include "gsm/constants.h"
#include <iostream>

GSM::SequenceEstimator::SequenceEstimator(
    GSM::StateMachine::Synchronize *ssm) {
  ssm_ = ssm;
  vd_ = new GSM::ViterbiDetector();
  decoder_ = new GSM::Decoder(ssm);

  gr_complex start_point;

  for (uint32_t i = 0; i < kNumTrainSeq; i++) {
    start_point = (kTrainingSequence[i][0] == 0) ?
        gr_complex(1.0, 0.0) : gr_complex(-1.0, 0.0);
    RotateSequence(kTrainingSequence[i],
                   kNumTrainBits,
                   normal_training_seq_[i],
                   start_point);
  }

  start_point = gr_complex(0.0, -1.0);

  RotateSequence(kSyncTrainingSequence,
                 kNumSyncBits,
                 SCH_training_seq_,
                 start_point);
}

gr_complex *GSM::SequenceEstimator::SCHTrainingSequence() {
  return SCH_training_seq_;
}

gr_complex *GSM::SequenceEstimator::NormalTrainingSequence(
    uint32_t bcc) {
  return normal_training_seq_[bcc];
}

// Estimate channel impulse response.
int32_t GSM::SequenceEstimator::EstimateImpulseResponse(
    const gr_complex *samples, std::string burst_type,
    float *max_correlation, gr_complex *estimated_cir) {
  std::vector<gr_complex> correlation_buffer;
  std::vector<float> power_buffer;
  std::vector<float> win_energy_buffer;

  int strongest_win_index;
  int burst_start = 0;
  int cir_center = 0;
  float energy = 0;

  int32_t search_start_pos, search_stop_pos,
      training_seq_length, training_seq_pos;
  gr_complex *training_seq;

  // Should support different training sequences based on BCC.
  if (!burst_type.compare("Synchronization")) {
    search_start_pos = kSyncPosition * kOSR;
    search_stop_pos = (kSyncPosition + kSyncSearchRange) * kOSR;
    training_seq_length = kNumSyncBits;
    training_seq_pos = kSyncPosition;
    training_seq = &SCHTrainingSequence()[5];

  } else if (!burst_type.compare("Normal")) {
    int32_t burst_center = round((kTrainPosition + kGuardPeriod) * kOSR);
    search_start_pos = burst_center + 1 - 5 * kOSR;
    search_stop_pos = burst_center + kCIRLength * kOSR + 5 * kOSR;
    training_seq_length = kNumTrainBits;
    training_seq_pos = kTrainPosition;
    training_seq = &NormalTrainingSequence(ssm_->bcc_)[5];

  } else if (!burst_type.compare("Dummy")) {
    int32_t burst_center = round((kTrainPosition + kGuardPeriod) * kOSR);
    search_start_pos = burst_center + 1 - 5 * kOSR;
    search_stop_pos = burst_center + kCIRLength * kOSR + 5 * kOSR;
    training_seq_length = kNumTrainBits;
    training_seq_pos = kTrainPosition;
    training_seq = &NormalTrainingSequence(kDummySequenceNum)[5];
  } else {
    return 0;
  }

  // Correlate training sequences at different lag and find the best
  // based on the energy. The correlation's index of the one with the
  // best energy corresponds to the index of that in the correlation
  // buffer.
  gr_complex correlation;
  for (int i = search_start_pos; i < search_stop_pos; i++) {
    // Cross-correlation:
    //   \sum\limits^{length}_{i = 0} \overline{f[i]} \cdot g[i + lag])
    //   (f, g, length, lag)
    correlation = CrossCorrelate(training_seq, &samples[0],
                                 training_seq_length - 10, i);

    correlation_buffer.push_back(correlation);
    power_buffer.push_back(std::pow(abs(correlation), 2));
  }

  // Iterate the power buffer and calculate the energy sum of a window
  // with the length of the desired channel impulse response. The
  // length must be between 1-5 to assure good results due to the
  // nature of training sequences that correlate to `0' with lag `-5 -
  // -1', `1 - 5' and to `16' with lag `0'.
  std::vector<float>::iterator window = power_buffer.begin();
  std::vector<float>::iterator slide;
  bool end_of_window = false;
  uint32_t i;
  while (window != power_buffer.end()) {
    // Set the slide to the start of the window initially.
    slide = window;
    energy = 0;

    for (i = 0; i < kCIRLength * kOSR; i++, slide++) {
      if (slide == power_buffer.end()) {
        // Break out, if we reach the end of the window.
        end_of_window = true;
        break;
      }
      energy += *slide;
    }

    if (end_of_window) {
      break;
    }

    win_energy_buffer.push_back(energy);

    window++;
  }

  // Find the index of the strongest window and fetch the
  // corresponding correlations and build the estimated impulse
  // response.
  strongest_win_index = max_element(
      win_energy_buffer.begin(), win_energy_buffer.end()) -
      win_energy_buffer.begin();

  if (strongest_win_index < 0) {
    strongest_win_index = 0;
  }

  *max_correlation = 0;

  for (uint32_t i = 0; i < kCIRLength * kOSR; i++) {
    correlation = correlation_buffer[strongest_win_index + i];
    if (abs(correlation) > *max_correlation) {
      cir_center = i;
      *max_correlation = abs(correlation);
    }

    estimated_cir[i] = correlation;
  }

  // Note this for synchronized state. It might not work.
  if (!burst_type.compare("Synchronization")) {
    burst_start = strongest_win_index + cir_center -
        ((training_seq_length - 10) - 2 * 3) * kOSR - 2 * kOSR +
        2 + training_seq_pos * kOSR;
  } else {
    burst_start = search_start_pos + strongest_win_index - kTrainPosition * kOSR;
  }

  // Short description from GSMSim documentation:
  //
  //   The last burst start calculation compensates for a delay
  //   inherently introduced in the transmitter as a result of the
  //   shaping operation of GMSK. As each bit is stretched over a
  //   period of 3T_b – with it’s maximum phase contribution in the
  //   last bit period – a delay of 2 T_b is expected. This
  //   corresponds to burst start being misplaced by OSR which then is
  //   corrected in the above code.
  return burst_start;
}

bool GSM::SequenceEstimator::Estimate(const gr_complex *samples,
                                      uint32_t num_samples,
                                      std::string burst_type) {
  uint32_t sample_num = ssm_->sample_number();

  // Static for now
  if (num_samples - sample_num < 500) {
    return false;
  }
  uint32_t i;

  float normal_max_correlation;
  gr_complex estimated_cir[kCIRLength * kOSR];
  gr_complex *cir = estimated_cir;

  int32_t burst_start = EstimateImpulseResponse(&samples[0],
                                                burst_type,
                                                &normal_max_correlation,
                                                &estimated_cir[0]);

  if (!burst_type.compare("Normal")) {
    float dummy_max_correlation;
    gr_complex estimated_dummy_cir[kCIRLength * kOSR];
    int32_t dummy_burst_start = EstimateImpulseResponse(
        &samples[0], "Dummy", &dummy_max_correlation, &estimated_dummy_cir[0]);

    if (dummy_max_correlation > normal_max_correlation) {
      cir = estimated_dummy_cir;
      burst_start = dummy_burst_start;
    }
  }

  // Only sets sample number for later use. Doesn't actually update
  // the pointer.
  ssm_->set_sample_number(sample_num + burst_start);

  // Fetch previous estimated CIR


  gr_complex Rhh_tmp[kCIRLength * kOSR];
  gr_complex Rhh[kCIRLength];

  float output[kBurstSize];

  AutoCorrelate(cir, kCIRLength * kOSR, &Rhh_tmp[0]);

  for (i = 0; i < kCIRLength; i++) {
    Rhh[i] = conj(Rhh_tmp[i * kOSR]);
  }

  gr_complex filtered_burst[kBurstSize];
  FilterBurst(&samples[burst_start], cir, filtered_burst);

  vd_->Estimate(filtered_burst, Rhh, output);

  uint8_t encoded_burst[kBurstSize];
  for (i = 0; i < kBurstSize; i++) {
    encoded_burst[i] = (output[i] > 0);
  }

  return decoder_->Decode(&encoded_burst[0], burst_type);
}

void GSM::SequenceEstimator::FilterBurst(
    const gr_complex *samples,
    gr_complex *cir,
    gr_complex *filtered_burst) {
  uint32_t a, i, n;
  for (n = 0; n < kBurstSize; n++) {
    a = n * kOSR;
    filtered_burst[n] = 0;
    i = 0;

    while (i < kCIRLength * kOSR) {
      if ((a + i) >= kBurstSize * kOSR) {
        break;
      }
      filtered_burst[n] += samples[a + i] * cir[i];
      i++;
    }
  }
}
