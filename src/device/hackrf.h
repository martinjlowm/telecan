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


#ifndef DEVICE_HACKRF_H_
#define DEVICE_HACKRF_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <libhackrf/hackrf.h>
#include <gnuradio/gr_complex.h>
#include "device/circular_buffer.h"

const uint32_t kExternalAmplifierGain = 1;
const uint32_t kLowNoiseAmplifierGain = 20;
const uint32_t kVariableGainAmplifer = 20;

const uint32_t kUSBPacketSize = 2 * 16384;
const uint32_t kFlushSize = 512;

namespace Device {

class HackRF {
 public:
  explicit HackRF(float sample_rate);
  ~HackRF();

  void Start();
  void Stop();
  void SampleRate();
  int Tune(double freq);
  int Open();
  int Fill(uint32_t num_samples, uint32_t *overrun_i);
  int Flush();

  CircularBuffer *get_buffer();
  int set_freq_correction(int ppm);
  bool set_gain (int amp_gain, int lna_gain, int vga_gain);
  float sample_rate();

  static int RXCallback(hackrf_transfer * transfer);

  int rx_count_;
  uint16_t ubuf_[kUSBPacketSize];

 private:
  hackrf_device *dev_;
  CircularBuffer *buffer_;
  float sample_rate_;
  double center_freq_;
  pthread_mutex_t usage_mutex_;
  std::vector<gr_complex> lut_;
};

}  // namespace Device

#endif  // DEVICE_HACKRF_H_
