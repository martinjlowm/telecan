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
