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

#include <arfcn_freq.h>

#include "gsm/constants.h"

BaseAnalyzer::BaseAnalyzer(usrp_source *usrp, int band_indicator) {
  if (usrp->open(0) == -1) {
    fprintf(stderr, "Error: usrp_source::open\n");
    exit(-1);
  }
  if (!usrp->set_gain(AMP_GAIN, LNA_GAIN, VGA_GAIN)) {
    fprintf (stderr, "Error: usrp_source::set_gain\n");
    exit(-1);
  }

  scan_status_ = NOT_SCANNED;

  usrp_ = usrp;

  band_indicator_ = band_indicator;
}

int BaseAnalyzer::GetBandIndicator() {
  return band_indicator_;
}

std::map<int, double> BaseAnalyzer::GetAvailableChannels() {
  return channels_;
}

int BaseAnalyzer::GetCurrentChannel() {
  return current_channel_;
}

void BaseAnalyzer::SetCurrentChannel(int channel) {
  current_channel_ = channel;
}

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

void BaseAnalyzer::SetFrequency(double frequency) {
  channels_[current_channel_] = frequency;
}

usrp_source* BaseAnalyzer::GetPeripheralDevice() {
  return usrp_;
}

bool BaseAnalyzer::HasScanned() {
  return scan_status_ == HAS_SCANNED;
}

bool BaseAnalyzer::IsScanning() {
  return scan_status_ == IS_SCANNING;
}
