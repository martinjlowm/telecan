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

#include "gsm/state_machine/find_fcch.h"

GSM::StateMachine::FindFCCH::FindFCCH() {
  state_ = new Initial();
}

bool GSM::StateMachine::FindFCCH::IsFCCHFound() {
  return fcch_found_;
}

void GSM::StateMachine::FindFCCH::FoundFCCH(bool found_fcch) {
  fcch_found_ = found_fcch;
}

void GSM::StateMachine::FindFCCH::Initial::Execute(Base *ffsm) {
  ffsm->SetCurrent(new Initial());
  delete this;
}

void GSM::StateMachine::FindFCCH::Search::Execute(Base *ffsm) {
  ffsm->SetCurrent(new Initial());
  delete this;
}

void GSM::StateMachine::FindFCCH::FoundSomething::Execute(Base *ffsm) {
  ffsm->SetCurrent(new Initial());
  delete this;
}

void GSM::StateMachine::FindFCCH::FoundFCCH::Execute(Base *ffsm) {
  ffsm->SetCurrent(new Initial());
  delete this;
}

void GSM::StateMachine::FindFCCH::SearchFailed::Execute(Base *ffsm) {
  ffsm->SetCurrent(new Initial());
  delete this;
}
