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

#include <boost/circular_buffer.hpp>

#include "gsm/state_machine/synchronize.h"
#include "gsm/constants.h"
#include "helper_functions.h"

#include <iostream>


GSM::StateMachine::FindFCCH::FindFCCH(Synchronize *ssm) {
  ssm_ = ssm;
  state_ = new Initial();
  phase_diff_buffer_ = boost::circular_buffer<float>(kFCCHHitsNeeded);
}

bool GSM::StateMachine::FindFCCH::FCCH_found() {
  return FCCH_found_;
}

void GSM::StateMachine::FindFCCH::set_FCCH_found(bool FCCH_found) {
  FCCH_found_ = FCCH_found;
}

bool GSM::StateMachine::FindFCCH::searching() {
  return searching_;
}

void GSM::StateMachine::FindFCCH::set_searching(bool searching) {
  searching_ = searching;
}

void GSM::StateMachine::FindFCCH::set_computed_freq_offset(
    double freq_offset) {
  computed_freq_offset_ = freq_offset;
}

double GSM::StateMachine::FindFCCH::computed_freq_offset() {
  return computed_freq_offset_;
}

void GSM::StateMachine::FindFCCH::Initial::Execute(
    Base *sm,
    const gr_complex */* samples */,
    uint32_t /* num_samples */) {
  // std::cout << "Initial\n";
  FindFCCH *ffsm = static_cast<FindFCCH *>(sm);

  ffsm->hit_count_ = 0;
  ffsm->miss_count_ = 0;
  ffsm->start_position_ = -1;
  ffsm->min_extrema_phase_diff_ = 99999;
  ffsm->phase_diff_buffer_.clear();
  ffsm->best_sum_ = 0;
  ffsm->computed_freq_offset_ = 0;

  ffsm->set_state(new Search());
  delete this;
}

void GSM::StateMachine::FindFCCH::Search::Execute(
    Base *sm,
    const gr_complex *samples,
    uint32_t num_samples) {
  // std::cout << "Search\n";

  FindFCCH *ffsm = static_cast<FindFCCH *>(sm);
  Synchronize *ssm = static_cast<Synchronize *>(ffsm->ssm_);

  State *next_state = 0;

  float phase_diff;

  uint32_t sample_num = ssm->sample_number();
  sample_num++;

  if (sample_num > num_samples - kFCCHHitsNeeded * kOSR) {
    next_state = new SearchFailed();
  } else {
    phase_diff = ComputePhaseDifference(samples[sample_num],
                                        samples[sample_num - 1]);

    if (phase_diff > 0) {
      next_state = new FoundSomething();
      ffsm->hit_count_++;
    } else {
      next_state = new Search();
      ffsm->miss_count_++;
    }
  }

  ssm->set_sample_number(sample_num);

  if (next_state) {
    ffsm->set_state(next_state);
    delete this;
  }
}

void GSM::StateMachine::FindFCCH::FoundSomething::Execute(
    Base *sm,
    const gr_complex *samples,
    uint32_t num_samples) {
  // std::cout << "FoundSomething\n";
  FindFCCH *ffsm = static_cast<FindFCCH *>(sm);
  Synchronize *ssm = static_cast<Synchronize *>(ffsm->ssm_);
  State *next_state = 0;

  uint32_t sample_num = ssm->sample_number();

  uint32_t hit_count = ffsm->hit_count_;
  uint32_t miss_count = ffsm->miss_count_;
  float min_extrema_phase_diff = ffsm->min_extrema_phase_diff_;
  int start_pos = ffsm->start_position_;
  double best_sum = ffsm->best_sum_;

  float min_phase_diff;
  float max_phase_diff;

  boost::circular_buffer<float> phase_diff_buffer = ffsm->phase_diff_buffer_;
  boost::circular_buffer<float>::iterator buffer_iter;

  // Search for the frequency correction burst. It is a pure tone of
  // all zeros, which means it has a positive phase difference of the
  // duration of a burst. Symbols are mapped to rotations and 158
  // zeros equal to a sequence of [1, j, -1, -j, 1, ..., 1].
  if ((miss_count >= kFCCHMaxMisses * kOSR) &&
      (hit_count <= kFCCHHitsNeeded * kOSR)) {
    next_state = new Initial();
  } else if (((miss_count >= kFCCHMaxMisses * kOSR) &&
              (hit_count > kFCCHHitsNeeded * kOSR)) ||
             (hit_count > 2 * kFCCHHitsNeeded * kOSR)) {
    next_state = new FoundFCCH();
  } else if ((miss_count < kFCCHMaxMisses * kOSR) &&
             (hit_count > kFCCHHitsNeeded * kOSR)) {
    // We assume we're in the middle of a burst since the phase is
    // positive for a long time (almost no miss counts). We continue
    // to look at the phase difference and once we find a high
    // frequency at the end, (which we assume will have a huge drop in
    // phase difference) we find the FCCH start position based on the
    // hits and misses.  FCCH is in fact found in this state, but we
    // wait until the burst is over before proceeding.
    min_phase_diff = *(min_element(phase_diff_buffer.begin(),
                                   phase_diff_buffer.end()));
    max_phase_diff = *(max_element(phase_diff_buffer.begin(),
                                   phase_diff_buffer.end()));

    if (min_extrema_phase_diff > max_phase_diff - min_phase_diff) {
      min_extrema_phase_diff = max_phase_diff - min_phase_diff;
      // Start position of the FCCH is found by subtracting the start
      // misses from the sufficient hits from the current sample
      // number. This will lead to an approximate sample number of the
      // beginning of the frequency correction burst.
      start_pos = sample_num - kFCCHHitsNeeded * kOSR - kFCCHMaxMisses * kOSR;
      best_sum = 0;

      for (buffer_iter = phase_diff_buffer.begin();
           buffer_iter != (phase_diff_buffer.end());
           buffer_iter++) {
        best_sum += *buffer_iter - (M_PI / 2) / kOSR;
      }
    }
  }

  sample_num++;

  if (sample_num >= num_samples) {
    next_state = new SearchFailed();
  }

  float phase_diff = ComputePhaseDifference(samples[sample_num],
                                            samples[sample_num - 1]);
  // printf("sample_num: %d, Phase diff %8.f\n", sample_num, phase_diff);
  if (phase_diff > 0) {
    hit_count++;
  } else {
    miss_count++;
  }

  // printf("%d ", ffsm->hit_count_);
  // printf("%d\n", ffsm->miss_count_);

  phase_diff_buffer.push_back(phase_diff);

  ffsm->phase_diff_buffer_ = phase_diff_buffer;
  ffsm->min_extrema_phase_diff_ = min_extrema_phase_diff;
  ffsm->hit_count_ = hit_count;
  ffsm->miss_count_ = miss_count;
  ffsm->start_position_ = start_pos;
  ffsm->best_sum_ = best_sum;

  ssm->set_sample_number(sample_num);

  if (next_state) {
    ffsm->set_state(next_state);
    delete this;
  }
}

void GSM::StateMachine::FindFCCH::FoundFCCH::Execute(
    Base *sm,
    const gr_complex */* samples */,
    uint32_t /* num_samples */) {
  // std::cout << "FoundFCCH\n";
  FindFCCH *ffsm = static_cast<FindFCCH *>(sm);
  Synchronize *ssm = static_cast<Synchronize *>(ffsm->ssm_);

  int start_pos = ffsm->start_position_;

  ssm->set_FCCH_start_position(start_pos);

  double phase_offset = ffsm->best_sum_ / kFCCHHitsNeeded;
  double freq_offset = phase_offset * kGSMBitRate / (2 * M_PI);
  ffsm->set_computed_freq_offset(freq_offset);

  ffsm->set_searching(false);
  ffsm->set_FCCH_found(true);

  ffsm->set_state(new Initial());
  delete this;
}

void GSM::StateMachine::FindFCCH::SearchFailed::Execute(
    Base *sm,
    const gr_complex */* samples */,
    uint32_t num_samples) {
  // std::cout << "SearchFailed\n";
  FindFCCH *ffsm = static_cast<FindFCCH *>(sm);
  Synchronize *ssm = static_cast<Synchronize *>(ffsm->ssm_);

  // Think this through and figure out if this should be done here or
  // elsewhere.
  ssm->set_sample_number(0);

  ffsm->set_searching(false);
  ffsm->set_FCCH_found(false);

  ffsm->set_state(new Initial());
  delete this;
}
