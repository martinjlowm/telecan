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

// Code is based on GNU Radio.

#include "dsp/fractional_resampler.h"

DSP::FractionalResampler::FractionalResampler(float phase_shift,
                                              float sample_ratio) {
  phase_shift_ = phase_shift;
  resample_ratio_ = sample_ratio;
  resample_response_ = new gr::filter::mmse_fir_interpolator_cc();
}


uint32_t DSP::FractionalResampler::Resample(const gr_complex *in,
                                            uint32_t num_samples,
                                            gr_complex *out) {
  uint32_t ii = 0; // input index
  uint32_t oo = 0; // output index

  while(ii < num_samples) {
    out[oo++] = resample_response_->interpolate(&in[ii], phase_shift_);

    double s = phase_shift_ + resample_ratio_;
    double f = floor(s);
    int incr = round(f);
    phase_shift_ = s - f;
    ii += incr;
  }

  return oo;
}
