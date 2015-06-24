

#ifndef GSM_DECODER_H_
#define GSM_DECODER_H_

#include <stdint.h>

#include "gsm/parser.h"
#include "gsm/state_machine/synchronize.h"

namespace GSM {

class Parser;

class Decoder {
 public:
  explicit Decoder(GSM::StateMachine::Synchronize *ssm);
  ~Decoder();

  bool Decode(uint8_t *encoded_burst, std::string burst_type);

  int32_t Deconvolve(uint8_t *data,
                     uint8_t *output,
                     uint32_t conv_output_size,
                     uint32_t max_error);

  int32_t CheckParity(uint8_t *d,
                      uint8_t *parity_polynomial, uint8_t *parity_remainder,
                      uint32_t data_size, uint32_t parity_size);

 private:
  Parser *parser_;

  static uint8_t sync_parity_polynomial_[];
  static uint8_t sync_parity_remainder_[];
  static uint8_t normal_parity_polynomial_[];
  static uint8_t normal_parity_remainder_[];

  uint32_t interleave_table_[kNormalConvSize];

  GSM::StateMachine::Synchronize *ssm_;
};

}  // namespace GSM

#endif  // GSM_DECODER_H_
