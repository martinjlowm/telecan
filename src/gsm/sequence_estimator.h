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

#ifndef GSM_SEQUENCE_ESTIMATOR_H_
#define GSM_SEQUENCE_ESTIMATOR_H_

#include "gsm/state_machine/synchronize.h"

#include "gsm/constants.h"
#include "gsm/viterbi_detector.h"
#include "gsm/decoder.h"

namespace GSM {

class ViterbiDetector;
class Decoder;

class SequenceEstimator {
 public:
  explicit SequenceEstimator(GSM::StateMachine::Synchronize *ssm);

  gr_complex *SCHTrainingSequence();
  gr_complex *NormalTrainingSequence(uint32_t bcc);

  int32_t EstimateImpulseResponse(const gr_complex *samples,
                                  std::string burst_type,
                                  float *max_correlation,
                                  gr_complex *estimated_cir);

  bool Estimate(const gr_complex *samples,
                uint32_t num_samples,
                std::string burst_type);

  void FilterBurst(const gr_complex *samples, gr_complex *cir,
                   gr_complex *filtered_burst);

 protected:
  GSM::StateMachine::Synchronize *ssm_;
  GSM::ViterbiDetector *vd_;
  GSM::Decoder *decoder_;
  gr_complex cir_[kCIRLength * kOSR];

  gr_complex SCH_training_seq_[kNumSyncBits];
  gr_complex normal_training_seq_[kNumTrainSeq][kNumTrainBits];
};

}  // namespace GSM

#endif  // GSM_SEQUENCE_ESTIMATOR_H_
