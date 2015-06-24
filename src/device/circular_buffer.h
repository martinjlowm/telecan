/* -*- c++ -*- */
/*
 * Copyright (c) 2010, Joshua Lackey
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

/*
 * circular_buffer
 *
 * This class is based heavily on the GNU Radio circular buffer.  While this
 * class was written from scratch and contains ideas not present in the GNU
 * Radio implementation, the GNU Radio circular buffers were used as a
 * reference while developing this class.
 *
 * This is more a warning that the above BSD-style license may not be the only
 * copyright that applies.
 */

#ifndef DEVICE_CIRCULAR_BUFFER_H_
#define DEVICE_CIRCULAR_BUFFER_H_

/*
 * XXX If read doesn't catch up with write before 2**64 bytes are written, this
 * will break.
 */

#include <stdint.h>
#include <pthread.h>

namespace Device {

class CircularBuffer {
 public:
  CircularBuffer(const uint32_t buf_len, const uint32_t item_size =
                  1, const uint32_t overwrite = 0);
  ~CircularBuffer();

  uint32_t Read(void *buf, const uint32_t buf_len);
  void *Peek(uint32_t *buf_len);
  uint32_t Purge(const uint32_t buf_len);
  void *Poke(uint32_t *buf_len);
  void Wrote(uint32_t len);
  uint32_t Write(const void *buf, const uint32_t buf_len);
  uint32_t DataAvailable();
  uint32_t SpaceAvailable();
  void Flush();
  void FlushNoLock();
  void Lock();
  void Unlock();
  uint32_t BufferLength();

private:
  void *m_buf;
  uint32_t m_buf_len, m_buf_size, m_r, m_w, m_item_size;
  uint64_t m_read, m_written;

  uint32_t m_overwrite;

  void *m_base;
  uint32_t m_pagesize;

  pthread_mutex_t m_mutex;
};

}  // namespace Device

#endif  // DEVICE_CIRCULAR_BUFFER_H_
