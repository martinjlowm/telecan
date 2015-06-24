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

#include "device/hackrf.h"

Device::HackRF::HackRF(float sample_rate) {
  // Lookup table for normalized samples. Same format as GNU Radio
  for (unsigned int i = 0; i <= 0xffff; i++) {
    lut_.push_back( gr_complex(
        (float(char(i & 0xff))) * (1.0f/128.0f),
        (float(char(i >> 8))) * (1.0f/128.0f) ) );
  }

  sample_rate_ = sample_rate;
  buffer_ = new CircularBuffer(8 * kUSBPacketSize, sizeof(gr_complex), 0);
  pthread_mutex_init (&usage_mutex_, 0);
  hackrf_init();
}

Device::HackRF::~HackRF() {
  Stop();
  delete buffer_;
  hackrf_close(dev_);
  hackrf_exit();
}


void Device::HackRF::Stop() {
  int result;
  result = hackrf_stop_rx(dev_);
  if (result != HACKRF_SUCCESS) {
    printf ("hackrf_stop_rx() failed: %s (%d)\n",
            hackrf_error_name ((hackrf_error) result), result);
    exit (1);
  }
}

void Device::HackRF::Start() {
  int result;
  result = hackrf_start_rx (dev_, Device::HackRF::RXCallback, this);

  if (result != HACKRF_SUCCESS) {
    printf ("hackrf_start_rx() failed: %s (%d)\n",
            hackrf_error_name ((hackrf_error) result), result);
    pthread_mutex_unlock (&usage_mutex_);
    exit (1);
  }
}

float Device::HackRF::sample_rate() {
  return sample_rate_;
}

int Device::HackRF::Tune(double freq) {
  int r = 0;

  if (freq != center_freq_) {
    r = hackrf_set_freq(dev_, (uint64_t) freq);

    if (r < 0) {
      fprintf(stderr, "Tuning failed!\n");
    } else {
      center_freq_ = freq;
    }
  }

  return 1;
}

bool Device::HackRF::set_gain(int amp_gain,
                              int lna_gain,
                              int vga_gain) {
  int r = 0;

  lna_gain = (((lna_gain + 7) / 8) * 8);
  vga_gain = (((vga_gain + 1) / 2) * 2);

  if (lna_gain > 40)
    lna_gain = 40;
  if (vga_gain > 62)
    vga_gain = 62;

  if (amp_gain)
    r = hackrf_set_amp_enable (dev_, amp_gain);
  if (vga_gain)
    r |= hackrf_set_vga_gain (dev_, vga_gain);
  if (lna_gain)
    r |= hackrf_set_lna_gain (dev_, lna_gain);

  return (r < 0) ? 0 : 1;
}

int Device::HackRF::Open() {
  int r;

  r = hackrf_init();
  if (r != HACKRF_SUCCESS) {
    printf ("hackrf_init() failed.");
    return EXIT_FAILURE;
  }

  r = hackrf_open(&dev_);
  if (r != HACKRF_SUCCESS) {
    fprintf (stderr, "Failed to open hackrf device.\n");
    exit (1);
  }

  /* Set the sample rate */
  r = hackrf_set_sample_rate(dev_, sample_rate_);

  if (r != HACKRF_SUCCESS)
    fprintf (stderr, "WARNING: Failed to set sample rate.\n");

  r = hackrf_set_baseband_filter_bandwidth (dev_, 2500000);
  if (r != HACKRF_SUCCESS) {
    printf ("hackrf_baseband_filter_bandwidth_set() failed: \n");
    return EXIT_FAILURE;
  }

  return 0;
}

int Device::HackRF::RXCallback(hackrf_transfer *transfer) {
  Device::HackRF *dev;
  dev = reinterpret_cast<Device::HackRF *>(transfer->rx_ctx);

  size_t bytes_to_write;
  size_t rx_count_new = dev->rx_count_ + transfer->valid_length;

  int count_left = kUSBPacketSize - rx_count_new;
  if (count_left <= 0) {
    bytes_to_write = transfer->valid_length + count_left;
  } else {
    bytes_to_write = transfer->valid_length;
  }

  if (bytes_to_write != 0) {
    memcpy(reinterpret_cast<int8_t*>(dev->ubuf_) + dev->rx_count_, transfer->buffer, bytes_to_write);
    dev->rx_count_ = dev->rx_count_ + bytes_to_write;
  }

  return 0;
}

int Device::HackRF::Fill(uint32_t num_samples,
                         uint32_t *overrun_i) {
  uint32_t i, space, overruns = 0;
  gr_complex *c;
  int n_read;

  while ((buffer_->DataAvailable() < num_samples) &&
         (buffer_->SpaceAvailable () > 0)) {
    // read one usb packet from hackrf
    pthread_mutex_lock (&usage_mutex_);

    // fill ubuf with sizeof(ubuf) sample points. n_read to indicate samples actually wrote.
    rx_count_ = 0;	// clear counter
    while (hackrf_is_streaming(dev_) != HACKRF_TRUE) {
      printf ("waiting for streaming...(%d)\n",
              hackrf_is_streaming(dev_));
    }

    while (hackrf_is_streaming(dev_) == HACKRF_TRUE) {
      if (rx_count_ == kUSBPacketSize) {
        break;
      }
    }

    n_read = rx_count_;

    pthread_mutex_unlock (&usage_mutex_);

    // write complex<short> input to complex<float> output
    c = reinterpret_cast<gr_complex *>(buffer_->Poke(&space));

    space = n_read;

    // write data
    for (i = 0; i < space; i++) {
      // c[i] = gr_complex(ubuf_[j] * 256, ubuf_[j + 1] * 256);
      c[i] = lut_[ubuf_[i]];
    }

    // update cb
    buffer_->Wrote(i);
  }

  // if the cb is full, we left behind data from the usb packet
  if (buffer_->SpaceAvailable() == 0) {
    fprintf (stderr, "warning: local overrun\n");
    overruns++;
  }

  if (overrun_i) {
    *overrun_i = overruns;
  }

  return 0;
}

Device::CircularBuffer *Device::HackRF::get_buffer () {
  return buffer_;
}


int32_t Device::HackRF::Flush() {
  buffer_->Flush();

  return 0;
}
