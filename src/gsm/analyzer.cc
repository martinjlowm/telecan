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

#include "helper_functions.h"
#include "gsm/constants.h"
#include "gsm/state_machine/synchronize.h"

GSM::Analyzer::Analyzer(Device::HackRF *sdr, int band_indicator, bool scan_bts)
    : BaseAnalyzer(sdr, band_indicator) {
  ssm_ = new StateMachine::Synchronize(this);

  // Determine how many samples we wish to fetch.
  float samples_per_symbol = kSampleRate / kGSMBitRate;
  read_num_samples_ = (uint32_t) ceil((12 * kFrameLength + kBurstLength) * samples_per_symbol);

  sdr_->Start();

  resampler_ = new DSP::FractionalResampler::FractionalResampler(
      0, kSampleRate / (kGSMBitRate * kOSR));

  lpf_ = new gr::filter::kernel::fir_filter_ccf(
      1,
      gr::filter::firdes::low_pass(1,
                                   kGSMBitRate * kOSR,
                                   100e3,  // 100kHz should perform well, bandwidth is 200kHz
                                   5e3,
                                   gr::filter::firdes::WIN_HAMMING,
                                   6.76));

  if (scan_bts) {
    Scan();
  }
}

void GSM::Analyzer::Scan() {
  scan_status_ = IS_SCANNING;
  std::cout << "Scanning...\n";

  bool valid;
  uint8_t attempts;
  int i;
  for (i = first_chan(band_indicator_); i >= 0; i = next_chan(i, band_indicator_)) {
    std::cout << "\r  ARFCN: " << i << std::flush;
    set_current_channel(i);

    valid = true;
    attempts = 0;

    while (attempts < 3) {
      BaseAnalyzer::Analyze();
      valid = valid && ssm_->ffsm()->FCCH_found();
      attempts++;
    }

    if (valid && ssm_->ffsm()->FCCH_found()) {
      SetFrequency(arfcn_to_freq(current_channel_, &band_indicator_) +
                   ssm_->ffsm()->computed_freq_offset());
    }
  }
  std::cout << "\n";
  // usrp_->stop();  // Stop doesn't work?

  scan_status_ = HAS_SCANNED;
}

void GSM::Analyzer::Analyze(const gr_complex *samples,
                            uint32_t num_samples) {
  // Downsample samples
  gr_complex *resampled_samples = new gr_complex[num_samples];
  uint32_t num_resampled_samples;
  num_resampled_samples = resampler_->Resample(samples,
                                               num_samples,
                                               resampled_samples);

  // Pass samples through a LPF
  gr_complex *filtered_samples = new gr_complex[num_resampled_samples];
  lpf_->filterN(filtered_samples,
                resampled_samples,
                num_resampled_samples);

  delete [] resampled_samples;

  // Loop this until we need to fetch more samples. FCCH state machine
  // may inform us about this. Time and synchronized state as well.
  while (ssm_->sample_number() < num_resampled_samples) {
    ssm_->Execute(filtered_samples, num_resampled_samples);
  }

  ssm_->set_sample_number(0);

  delete [] filtered_samples;
}
