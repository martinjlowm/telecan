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

#ifndef BASE_ANALYZER_H_
#define BASE_ANALYZER_H_

#include "device/hackrf.h"

#include <map>

class BaseAnalyzer {
 public:
  BaseAnalyzer(Device::HackRF *sdr, int band_indicator);
  ~BaseAnalyzer();

  int32_t band_indicator();

  int32_t current_channel();
  void set_current_channel(int channel);
  std::map<int, double> GetAvailableChannels();

  double GetFrequency();
  void SetFrequency(double frequency);

  Device::HackRF* GetPeripheralDevice();

  bool HasScanned();
  bool IsScanning();
  bool AllConsumed(int32_t num_samples);
  void Consume(int32_t samples);

  void Analyze();
  virtual void Analyze(const gr_complex *samples,
                       uint32_t num_samples) = 0;
  virtual void Scan() = 0;

 protected:
  Device::HackRF *sdr_;
  int32_t band_indicator_;

  enum ScanStatus {
    NOT_SCANNED,
    HAS_SCANNED,
    IS_SCANNING
  } scan_status_;

  int current_channel_;
  std::map<int, double> channels_;
  uint32_t read_num_samples_;
};

#endif  // BASE_ANALYZER_H_
