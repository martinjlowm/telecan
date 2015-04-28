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

#include "gsm/analyzer.h"

#include <iostream>
#include <arfcn_freq.h>

#include "gsm/state_machine/synchronize.h"

GSM::Analyzer::Analyzer(usrp_source *usrp, int band_indicator, bool scan_bts)
    : BaseAnalyzer(usrp, band_indicator) {
  ssm_ = new StateMachine::Synchronize(this);

  usrp_->start();

  if (scan_bts) {
    Scan();
  }
}

// TODO: Rethink this later. Should it be possible to change frequency
// during runtime?
// void GSM::Analyzer::SetCurrentChannel(int channel) {
//   BaseAnalyzer::SetCurrentChannel(channel);

//   StateMachine::State *current_state = ssm_->GetState();
//   ssm_->SetState(new StateMachine::Synchronize::FCCHSearch());
//   delete current_state;
// }

void GSM::Analyzer::Scan() {
  scan_status_ = IS_SCANNING;
  std::cout << "Scanning...\n";
  // usrp_->start();
  int i;
  for (i = first_chan(band_indicator_); i >= 0; i = next_chan(i, band_indicator_)) {
    std::cout << "\r  ARFCN: " << i << std::flush;
    SetCurrentChannel(i);
    ssm_->Execute();
  }
  std::cout << "\n";
  // usrp_->stop();  // Stop doesn't work?

  scan_status_ = HAS_SCANNED;
}

void GSM::Analyzer::Analyze() {
  // usrp_->start();
  while (1) {
    ssm_->Execute();
  }
  // usrp_->stop();
}
