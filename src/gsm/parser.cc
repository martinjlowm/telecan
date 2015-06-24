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
#include "gsm/parser.h"

const std::string pd_values[] = {
    "Group Call Control",
    "Broadcast Call Control",
    "PDSS1",
    "Call Control",
    "PDSS2",
    "Mobility Management Messages",
    "Radio Resources Management Messages",
    "GPRS Mobility Management Messages",
    "SMS Messages",
    "GPRS Session Management Messages",
    "Non-Call Related Supplementary Service Messages",
    "Location Services",
    "--Reserved--",
    "--Reserved For Tests--"
};

const std::string mt_values[] = {
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","",
  "IMMEDIATE SETUP",
  "SETUP",
  "CONNECT",
  "TERMINATION",
  "TERMINATION REQUEST",
  "TERMINATION REJECT","",
  "STATUS",
  "GET STATUS",
  "SET STATUS"
};

GSM::Parser::Parser() {
  ssm_ = 0;

  Initialize();
}

GSM::Parser::Parser(GSM::StateMachine::Synchronize *ssm) {
  ssm_ = ssm;

  Initialize();
}

void GSM::Parser::Initialize() {
  frame_ = std::vector<uint8_t>();

  parser_["Normal"] = GSM::Parser::Normal;
  parser_["Synchronization"] = GSM::Parser::Synchronization;
}

void GSM::Parser::Parse(const uint8_t *frame, uint32_t length,
                        std::string burst_type) {
  if (!parser_[burst_type]) {
    return;
  }

  output_.str("");

  frame_ = std::vector<uint8_t>(length);
  for (uint32_t i = 0; i < length; i++) {
    frame_[i] = frame[i];
  }

  output_ << burst_type << ":\n";
  parser_[burst_type](frame_, output_, ssm_);

  std::cout << output_.str() << "\n";
}


void GSM::Parser::Normal(std::vector<uint8_t> &frame,
                         std::stringstream &output,
                         GSM::StateMachine::Synchronize *ssm) {
  uint32_t octet = 1;
  // If Bbis format:
  // http://www.etsi.org/deliver/etsi_ts/100900_100999/100940/07.21.00_60/ts_100940v072100p.pdf 04.08
  uint32_t l2pl = (uint32_t)
      (frame[OFFSET(octet) + 7] << 5) |
      (frame[OFFSET(octet) + 6] << 4) |
      (frame[OFFSET(octet) + 5] << 3) |
      (frame[OFFSET(octet) + 4] << 2) |
      (frame[OFFSET(octet) + 3] << 1) |
      (frame[OFFSET(octet) + 2] << 0);

  output << boost::format("  Pseudo Length: %d\n") % l2pl;

  if (l2pl == 0) {
    return;
  }

  octet++;
  // Protocol discriminator
  uint32_t pd = (uint32_t)
      (frame[OFFSET(octet) + 3] << 3) |
      (frame[OFFSET(octet) + 2] << 2) |
      (frame[OFFSET(octet) + 1] << 1) |
      (frame[OFFSET(octet) + 0] << 0);

  output << boost::format(
      "  Protocol Discriminator: %d (%s)\n") % pd % pd_values[pd];
  // Transaction identifier
  uint32_t ti = (uint32_t)
      (frame[OFFSET(octet) + 7] << 3) |
      (frame[OFFSET(octet) + 6] << 2) |
      (frame[OFFSET(octet) + 5] << 1) |
      (frame[OFFSET(octet) + 4] << 0);

  output << boost::format("  Transaction Identifier: %d\n") % ti;

  octet++;
  // Message type
  // E.g.
  // 0x110001 49 IMMEDIATE SETUP
  // 0x110010 50 SETUP
  // 0x110011 51 CONNECT
  // 0x110100 52 TERMINATION
  // 0x110101 53 TERMINATION REQUEST
  // 0x110110 54 TERMINATION REJECT
  // 0x111000 56 STATUS
  // 0x111001 57 GET STATUS
  // 0x111010 58 SET STATUS
  uint32_t mt = (uint32_t)
      (0 << 7) |  // always 0
      // (frame[OFFSET(octet) + 1] << 6) | // Reserved for send seq num
      (frame[OFFSET(octet) + 2] << 5) |
      (frame[OFFSET(octet) + 3] << 4) |
      (frame[OFFSET(octet) + 4] << 3) |
      (frame[OFFSET(octet) + 5] << 2) |
      (frame[OFFSET(octet) + 6] << 1) |
      (frame[OFFSET(octet) + 7] << 0);

  output << boost::format(
      "  Message Type: %d (%s)\n") % mt % mt_values[mt];
  output << "  ...\n";
}

void GSM::Parser::FrequencyCorrection(std::vector<uint8_t> &frame,
                                      std::stringstream &output,
                                      GSM::StateMachine::Synchronize *ssm) {

  output << "Frequency Correction: ";

}

void GSM::Parser::Synchronization(std::vector<uint8_t> &frame,
                                  std::stringstream &output,
                                  GSM::StateMachine::Synchronize *ssm) {
  uint32_t octet = 1;
  // Network color code
  uint32_t ncc = (uint32_t)
      (frame[OFFSET(octet) + 7] << 2)  |
      (frame[OFFSET(octet) + 6] << 1)  |
      (frame[OFFSET(octet) + 5] << 0);

  // Base station color code
  uint32_t bcc = (uint32_t)
      (frame[OFFSET(octet) + 4] << 2)  |
      (frame[OFFSET(octet) + 3] << 1)  |
      (frame[OFFSET(octet) + 2] << 0);
  output << boost::format("  BSIC: (%d/%d)\n") % ncc % bcc;

  octet++;
  uint32_t T1 = (uint32_t)
      (frame[OFFSET(octet - 1) + 1] << 10) |
      (frame[OFFSET(octet - 1) + 0] << 9)  |
      (frame[OFFSET(octet) + 7] << 8)        |
      (frame[OFFSET(octet) + 6] << 7)        |
      (frame[OFFSET(octet) + 5] << 6)        |
      (frame[OFFSET(octet) + 4] << 5)        |
      (frame[OFFSET(octet) + 3] << 4)        |
      (frame[OFFSET(octet) + 2] << 3)        |
      (frame[OFFSET(octet) + 1] << 2)        |
      (frame[OFFSET(octet) + 0] << 1)        |
      (frame[OFFSET(octet + 1) + 7] << 0);
  output << boost::format("  T1: %d\n") % T1;

  octet++;
  uint32_t T2 = (uint32_t)
      (frame[OFFSET(octet) + 6] << 4)  |
      (frame[OFFSET(octet) + 5] << 3)  |
      (frame[OFFSET(octet) + 4] << 2)  |
      (frame[OFFSET(octet) + 3] << 1)  |
      (frame[OFFSET(octet) + 2] << 0);
  output << boost::format("  T2: %d\n") % T2;

  octet++;
  uint32_t T3 = (uint32_t)
      (frame[OFFSET(octet - 1) + 1] << 2)  |
      (frame[OFFSET(octet - 1) + 0] << 1)  |
      (frame[OFFSET(octet) + 0] << 0);

  if (ssm) {
    ssm->SetSynchronizationVariables(ncc, bcc, T1, T2, T3);
  }

  output << boost::format("  T3': %d\n") % T3;
}

void GSM::Parser::Dummy(std::vector<uint8_t> &frame,
                        std::stringstream &output,
                        GSM::StateMachine::Synchronize *ssm) {

  output << "";

}

void GSM::Parser::Access(std::vector<uint8_t> &frame,
                         std::stringstream &output,
                         GSM::StateMachine::Synchronize *ssm) {

  output << "";

}
