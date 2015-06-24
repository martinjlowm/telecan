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

#include "helper_functions.h"
#include <util.h>

#include "gsm/analyzer.h"
#include "gsm/constants.h"

#include <iostream>
#include <cstring>

GSM::StateMachine::Synchronize::Synchronize(GSM::Analyzer *analyzer) {
  analyzer_ = analyzer;

  ffsm_ = new StateMachine::FindFCCH(this);
  state_ = new FCCHSearch();

  seq_estimator_ = new SequenceEstimator(this);
  burst_counter_ = new BurstCounter();

  FCCH_start_position_ = 0;
  sample_number_ = 0;
  failures_ = 0;
}

GSM::StateMachine::FindFCCH* GSM::StateMachine::Synchronize::ffsm() {
  return ffsm_;
}

int GSM::StateMachine::Synchronize::FCCH_start_position() {
  return FCCH_start_position_;
}

void GSM::StateMachine::Synchronize::set_FCCH_start_position(int start_pos) {
  FCCH_start_position_ = start_pos;
}

void GSM::StateMachine::Synchronize::SetSynchronizationVariables(
    uint32_t ncc, uint32_t bcc,
    uint32_t T1, uint32_t T2, uint32_t T3a) {
  ncc_ = ncc;  // Perhaps useful at some point
  bcc_ = bcc;

  uint32_t T3 = 10 * T3a + 1;

  burst_counter_->Set(T1, T2, T3, 0);
}

void GSM::StateMachine::Synchronize::set_sample_number(
    uint32_t sample_number) {
  sample_number_ = sample_number;
}
uint32_t GSM::StateMachine::Synchronize::sample_number() {
  return sample_number_;
}

void GSM::StateMachine::Synchronize::FCCHSearch::Execute(
    Base *sm,
    const gr_complex *samples,
    uint32_t num_samples) {
  Synchronize *ssm = static_cast<Synchronize *>(sm);

  Analyzer *analyzer = ssm->analyzer_;
  Device::HackRF *sdr = analyzer->GetPeripheralDevice();

  ssm->ffsm_->set_searching(true);
  ssm->ffsm_->set_FCCH_found(false);

  while (ssm->ffsm_->searching()) {
    ssm->ffsm_->Execute(samples, num_samples);
  }

  if (ssm->ffsm_->FCCH_found()) {
    analyzer->SetFrequency(analyzer->GetFrequency() + ssm->ffsm_->computed_freq_offset());
    sdr->Tune(analyzer->GetFrequency());
  } else {
    ssm->failures_++;

    if (ssm->failures_ > kMaxErrors) {
      ssm->set_sample_number(num_samples);
      ssm->failures_ = 0;
    }
  }

  if (ssm->ffsm_->FCCH_found() && !analyzer->IsScanning()) {
    // std::cout << "FCCH found!\n";
    ssm->set_state(new SCHSearch());
    delete this;
  }
}

// Start implementing SCHSearch. We don't consume anything, eh GNU
// Radio. Increment FCCH start pos with estimated frame bits and
// safety margin and hope for the best.

void GSM::StateMachine::Synchronize::SCHSearch::Execute(
    Base *sm,
    const gr_complex *samples,
    uint32_t num_samples) {
  Synchronize *ssm = static_cast<Synchronize *>(sm);
  State *next_state = 0;
  bool decoded = 0;

  // Jump ahead to an approximate SCH position
  ssm->set_sample_number(ssm->FCCH_start_position() +
                         (kFrameBits - kSafetyMargin) * kOSR);

  // Pass pointer to the known burst start.
  decoded = ssm->seq_estimator_->Estimate(&samples[ssm->sample_number()],
                                          num_samples, "Synchronization");

  if (decoded) {
    (*ssm->burst_counter_)++;
    ssm->set_sample_number(ssm->sample_number() + (kBurstSize + 4) * kOSR);
    next_state = new Synchronized();
  } else {
    // Force FCCH search with new samples.
    ssm->set_sample_number(num_samples);
    next_state = new FCCHSearch();
  }

  ssm->set_state(next_state);
  delete this;
}

void GSM::StateMachine::Synchronize::Synchronized::Execute(
    Base *sm,
    const gr_complex *samples,
    uint32_t num_samples) {
  Synchronize *ssm = static_cast<Synchronize *>(sm);
  State *next_state = 0;
  bool decoded = 0;
  GSM::BurstCounter::burst_type burst_type = ssm->burst_counter_->GetType();

  switch (burst_type) {
    case GSM::BurstCounter::NORMAL_OR_DUMMY: {
      decoded = ssm->seq_estimator_->Estimate(&samples[ssm->sample_number()],
                                              num_samples, "Normal");

      if (!decoded && (num_samples - ssm->sample_number() < 500)) {
        ssm->failures_++;

        if (ssm->failures_ > kMaxErrors) {
          ssm->set_sample_number(num_samples);
          ssm->failures_ = 0;
        }
      }
      break;
    }
    case GSM::BurstCounter::SYNCHRONIZATION: {
      decoded = ssm->seq_estimator_->Estimate(&samples[ssm->sample_number()],
                                              num_samples, "Synchronization");

      if (decoded) {
        ssm->failures_ = 0;
        ssm->set_sample_number(ssm->sample_number() - floor(kGuardPeriod * kOSR));
      } else {
        ssm->failures_++;

        if (ssm->failures_ > kMaxErrors) {
          ssm->set_sample_number(num_samples);
          ssm->failures_ = 0;
          next_state = new FCCHSearch();
        }
      }
      break;
    }
    case GSM::BurstCounter::FREQUENCY_CORRECTION: {

      break;
    }
    case GSM::BurstCounter::ACCESS: {
      // Ignore for now
      break;
    }
    default: {
      break;
    }
  }

  (*ssm->burst_counter_)++;

  ssm->set_sample_number(ssm->sample_number() + kTimeslotBits * kOSR +
      ssm->burst_counter_->offset_integer());

  if (next_state) {
    ssm->set_state(next_state);
    delete this;
  }
}
