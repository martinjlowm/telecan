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

#ifndef GSM_STATE_MACHINE_FIND_FCCH_H_
#define GSM_STATE_MACHINE_FIND_FCCH_H_

#include "gsm/state_machine/state.h"
#include "gsm/state_machine/base.h"

#include <boost/circular_buffer.hpp>

namespace GSM {
namespace StateMachine {

class Synchronize;

class FindFCCH : public Base {
 public:
  explicit FindFCCH(Synchronize *ssm);

  bool FCCH_found();
  void set_FCCH_found(bool found_fcch);
  bool searching();
  void set_searching(bool found_fcch);
  double computed_freq_offset();
  void set_computed_freq_offset(double freq_offset);

  // Inital-state.
  class Initial : public State {
   public:
    void Execute(Base *ffsm,
                 const gr_complex *samples,
                 uint32_t num_samples);
  };
  friend class Initial;

  // Search-state.
  class Search : public State {
   public:
    void Execute(Base *ffsm,
                 const gr_complex *samples,
                 uint32_t num_samples);
  };
  friend class Search;

  // Something is found-state.
  class FoundSomething : public State {
   public:
    void Execute(Base *ffsm,
                 const gr_complex *samples,
                 uint32_t num_samples);
  };
  friend class FoundSomething;

  // FCCH is found-state.
  class FoundFCCH : public State {
   public:
    void Execute(Base *ffsm,
                 const gr_complex *samples,
                 uint32_t num_samples);
  };
  friend class FoundFCCH;

  // Search failed-state.
  class SearchFailed : public State {
   public:
    void Execute(Base *ffsm,
                 const gr_complex *samples,
                 uint32_t num_samples);
  };
  friend class SearchFailed;

 private:
  Synchronize *ssm_;
  bool FCCH_found_;
  bool searching_;

  uint32_t hit_count_;
  uint32_t miss_count_;
  int32_t start_position_;
  double best_sum_;
  int32_t min_extrema_phase_diff_;
  boost::circular_buffer<float> phase_diff_buffer_;
  double computed_freq_offset_;
};
}  // namespace StateMachine
}  // namespace GSM

#endif  // GSM_STATE_MACHINE_FIND_FCCH_H_
