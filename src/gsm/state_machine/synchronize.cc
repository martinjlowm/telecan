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

#include "gsm/state_machine/synchronize.h"

#include "gsm/constants.h"

GSM::StateMachine::Synchronize::Synchronize() {
  ffsm_ = new StateMachine::FindFCCH();
  bsm_ = new StateMachine::Burst();
  state_ = new FCCHSearch();
}

void GSM::StateMachine::Synchronize::FCCHSearch::Execute(Base *sync_state_machine) {
  Synchronize *ssm  = (Synchronize *) sync_state_machine;

  int channel = ssm->analyzer_->GetCurrentChannel();
  usrp_source *device = ssm->analyzer_->GetPeripheralDevice();
  int band_indicator = ssm->analyzer_->GetBandIndicator();

  double samples_per_second = device->sample_rate () / GSM_RATE;
  frames_len = (unsigned int) ceil ((12 * 8 * 156.25 + 156.25) * sps);
  ub = u->get_buffer ();

  sync_sm->ffsm_->Execute(b, b_len, &offset, 0);

  if (sync_sm->ffsm_->IsFCCHFound() && (fabsf (offset - GSM_RATE / 4) < ERROR_DETECT_OFFSET_MAX))
  if (foundFCCH) {
    channels_[i] = frequency;
    i = next_chan(i, band_indicator);
  } else {
    ssm->SetCurrent(new SCHSearch());
    delete this;
  }
}

void GSM::StateMachine::Synchronize::SCHSearch::Execute(Base *ssm) {
  ssm->SetCurrent(new FCCHSearch());
  delete this;
}

void GSM::StateMachine::Synchronize::Synchronized::Execute(Base *ssm) {
  ssm->SetCurrent(new FCCHSearch());
  delete this;
}
