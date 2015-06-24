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

#ifndef GSM_PARSER_H_
#define GSM_PARSER_H_

#include <boost/format.hpp>
#include <iostream>

#include "helper_functions.h"
#include "gsm/state_machine/synchronize.h"
#include "map"
#include "vector"

#define OFFSET(octet) (octet - 1) * 8

typedef void(*BurstHandler) (std::vector<uint8_t> &frame,
                             std::stringstream &output,
                             GSM::StateMachine::Synchronize *ssm);

// Protocol Discriminator values
extern const std::string pd_values[];

// Message types
extern const std::string mt_values[];

namespace GSM {

class Parser {
 public:
  Parser();
  explicit Parser(GSM::StateMachine::Synchronize *ssm);

  void Initialize();

  void Parse(const uint8_t *frame, uint32_t length,
             std::string channel);

  static void Normal(std::vector<uint8_t> &frame,
                     std::stringstream &output,
                     GSM::StateMachine::Synchronize *ssm);
  static void FrequencyCorrection(std::vector<uint8_t> &frame,
                                  std::stringstream &output,
                                  GSM::StateMachine::Synchronize *ssm);
  static void Synchronization(std::vector<uint8_t> &frame,
                              std::stringstream &output,
                              GSM::StateMachine::Synchronize *ssm);
  static void Dummy(std::vector<uint8_t> &frame,
                    std::stringstream &output,
                    GSM::StateMachine::Synchronize *ssm);
  static void Access(std::vector<uint8_t> &frame,
                     std::stringstream &output,
                     GSM::StateMachine::Synchronize *ssm);

  std::stringstream output_;
  std::vector<uint8_t> frame_;
  std::map<std::string, BurstHandler> parser_;
  GSM::StateMachine::Synchronize *ssm_;

 protected:

 private:

};

}  // namespace GSM

#endif  // GSM_PARSER_H_
