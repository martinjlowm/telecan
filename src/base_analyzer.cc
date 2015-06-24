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

#include "base_analyzer.h"

#include "helper_functions.h"
#include "device/hackrf.h"

#include "gsm/constants.h"

BaseAnalyzer::BaseAnalyzer(Device::HackRF *sdr, int band_indicator) {
  if (sdr->Open() == -1) {
    fprintf(stderr, "Error: SDR::Open\n");
    exit(-1);
  }
  if (!sdr->set_gain(kExternalAmplifierGain,
                     kLowNoiseAmplifierGain,
                     kVariableGainAmplifer)) {
    fprintf (stderr, "Error: SDR::set_gain\n");
    exit(-1);
  }

  scan_status_ = NOT_SCANNED;

  sdr_ = sdr;

  band_indicator_ = band_indicator;
}
BaseAnalyzer::~BaseAnalyzer() {}

int32_t BaseAnalyzer::band_indicator() { return band_indicator_; }


// Channel
int BaseAnalyzer::current_channel() { return current_channel_; }
void BaseAnalyzer::set_current_channel(int channel) { current_channel_ = channel; }
std::map<int, double> BaseAnalyzer::GetAvailableChannels() { return channels_; }


// Frequency
double BaseAnalyzer::GetFrequency() {
  double frequency;
  std::map<int, double>::iterator channel_iterator = channels_.find(current_channel_);

  if (channel_iterator == channels_.end()) {
    frequency = arfcn_to_freq(current_channel_, &band_indicator_);
  } else {
    frequency = channel_iterator->second;
  }

  return frequency;
}
void BaseAnalyzer::SetFrequency(double frequency) { channels_[current_channel_] = frequency; }


// Get SDR interface
Device::HackRF* BaseAnalyzer::GetPeripheralDevice() { return sdr_; }


// Scan status
bool BaseAnalyzer::HasScanned() { return scan_status_ == HAS_SCANNED; }
bool BaseAnalyzer::IsScanning() { return scan_status_ == IS_SCANNING; }


void BaseAnalyzer::Analyze() {
  uint32_t new_overruns, overruns = 0;

  Device::CircularBuffer *buffer = sdr_->get_buffer();
  gr_complex *samples;

  uint32_t num_samples;

  double frequency = GetFrequency();

  if (!sdr_->Tune(frequency)) {
    exit(-1);
  }

  do {
    sdr_->Flush();

    do {
      // Attempt to fetch 12 frames and one burst. Rethink this, X
      // samples are fetched and downsampled to kOSR times GSM
      // bitrate.

      if (sdr_->Fill(read_num_samples_, &new_overruns)) {
        exit(-1);
      }

      if (new_overruns) {
        overruns += new_overruns;
        sdr_->Flush();
      }
    } while (new_overruns);

    // Fetch samples and the number which was actually read.
    samples = static_cast<gr_complex *>(buffer->Peek(&num_samples));

    Analyze(samples, num_samples);

    // ssm_->Execute(samples, num_samples);
  } while (scan_status_ != IS_SCANNING);
}
