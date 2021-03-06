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

#ifndef GSM_ANALYZER_H_
#define GSM_ANALYZER_H_

#include "base_analyzer.h"

#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter.h>

#include "dsp/fractional_resampler.h"
#include "device/hackrf.h"

namespace GSM {
namespace StateMachine {

class Synchronize;

} // namespace StateMachine

class Analyzer : public BaseAnalyzer {
 public:
  Analyzer(Device::HackRF *sdr, int band_indicator, bool scan_bts);
  ~Analyzer();

  void Analyze(const gr_complex *samples,
               uint32_t num_samples);
  void Scan();

  DSP::FractionalResampler *resampler_;
  gr::filter::kernel::fir_filter_ccf *lpf_;
 private:
  StateMachine::Synchronize *ssm_;
};

}  // namespace GSM

#endif  // GSM_ANALYZER_H_
