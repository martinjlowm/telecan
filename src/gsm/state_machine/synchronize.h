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

#ifndef GSM_STATE_MACHINE_SYNCHRONIZE_H_
#define GSM_STATE_MACHINE_SYNCHRONIZE_H_

#include "gsm/state_machine/base.h"
#include "gsm/state_machine/state.h"
#include "gsm/state_machine/burst.h"
#include "gsm/state_machine/find_fcch.h"

namespace GSM {

class Analyzer;

namespace StateMachine {

class Synchronize : public Base {
 public:
  Synchronize();

  class FCCHSearch : public State {
   public:
    void Execute(Base *ssm);
  };
  friend class FCCHSearch;

  class SCHSearch : public State {
   public:
    void Execute(Base *ssm);
  };
  friend class FCCHSearch;

  class Synchronized : public State {
   public:
    void Execute(Base *ssm);
  };
  friend class FCCHSearch;

 protected:
  GSM::Analyzer *analyzer_;

 private:
  StateMachine::FindFCCH *ffsm_;
  StateMachine::Burst *bsm_;
};

}  // namespace StateMachine
}  // namespace GSM
#endif  // GSM_STATE_MACHINE_SYNCHRONIZE_H_
