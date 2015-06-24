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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "circular_buffer.h"

/*
 * OSX doesn't support System V shared memory.  Using GNU Radio as an example,
 * we'll implement this for OSX using Posix shared memory.  I'm not exactly
 * sure why GNU Radio prefers the System V usage, but I seem to recall there
 * was a reason.
 */
Device::CircularBuffer::CircularBuffer(const unsigned int buf_len,
                                       const unsigned int item_size,
                                       const unsigned int overwrite) {
  int shm_fd;
  char shm_name[255];		// XXX should be NAME_MAX
  void *base;

  if (!buf_len)
    throw std::runtime_error ("CircularBuffer: buffer len is 0");

  if (!item_size)
    throw std::runtime_error ("CircularBuffer: item size is 0");

  // calculate buffer size
  m_item_size = item_size;
  m_buf_size = item_size * buf_len;

  m_pagesize = getpagesize ();
  if (m_buf_size % m_pagesize)
    m_buf_size = (m_buf_size + m_pagesize) & ~(m_pagesize - 1);
  m_buf_len = m_buf_size / item_size;

  // create unique-ish name
  snprintf (shm_name, sizeof (shm_name), "/kalibrate-%d", getpid ());

  // create a Posix shared memory object
  if ((shm_fd =
       shm_open (shm_name, O_RDWR | O_CREAT | O_EXCL,
		 S_IRUSR | S_IWUSR)) == -1) {
    perror ("shm_open");
    throw std::runtime_error ("CircularBuffer: shm_open");
  }

  // create enough space to hold everything
  if (ftruncate (shm_fd, 2 * m_pagesize + 2 * m_buf_size) == -1) {
    perror ("ftruncate");
    close (shm_fd);
    shm_unlink (shm_name);
    throw std::runtime_error ("CircularBuffer: ftruncate");
  }

  // get an address for the buffer
  if ((base =
       mmap (0, 2 * m_pagesize + 2 * m_buf_size, PROT_NONE, MAP_SHARED,
	     shm_fd, 0)) == MAP_FAILED) {
    perror ("mmap");
    close (shm_fd);
    shm_unlink (shm_name);
    throw std::runtime_error ("CircularBuffer: mmap (base)");
  }

  // unmap everything but the first guard page
  if (munmap (reinterpret_cast<char *>(base) + m_pagesize,
              m_pagesize + 2 * m_buf_size) == -1) {
    perror ("munmap");
    close (shm_fd);
    shm_unlink (shm_name);
    throw std::runtime_error ("CircularBuffer: munmap");
  }

  // race condition

  // map first copy of the buffer
  if (mmap(reinterpret_cast<char *>(base) + m_pagesize,
           m_buf_size,
           PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
           shm_fd, m_pagesize) == MAP_FAILED) {
    perror ("mmap");
    munmap (base, 2 * m_pagesize + 2 * m_buf_size);
    close (shm_fd);
    shm_unlink (shm_name);
    throw std::runtime_error ("CircularBuffer: mmap (buf 1)");
  }

  // map second copy of the buffer
  if (mmap(reinterpret_cast<char *>(base) + m_pagesize + m_buf_size,
           m_buf_size,
           PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, shm_fd,
           m_pagesize) == MAP_FAILED) {
    perror ("mmap");
    munmap (base, 2 * m_pagesize + 2 * m_buf_size);
    close (shm_fd);
    shm_unlink (shm_name);
    throw std::runtime_error ("CircularBuffer: mmap (buf 2)");
  }

  // map second copy of the guard page
  if (mmap(reinterpret_cast<char *>(base) + m_pagesize + 2 * m_buf_size,
           m_pagesize, PROT_NONE, MAP_SHARED | MAP_FIXED,
           shm_fd, 0) == MAP_FAILED) {
    perror ("mmap");
    munmap (base, 2 * m_pagesize + 2 * m_buf_size);
    close (shm_fd);
    shm_unlink (shm_name);
    throw std::runtime_error ("CircularBuffer: mmap (guard)");
  }

  // both the file and name are unnecessary now
  close (shm_fd);
  shm_unlink (shm_name);

  // save the base address for unmap later
  m_base = base;

  // save a pointer to the data
  m_buf = reinterpret_cast<char *>(base) + m_pagesize;

  m_r = m_w = 0;
  m_read = m_written = 0;

  m_item_size = item_size;

  m_overwrite = overwrite;

  pthread_mutex_init (&m_mutex, 0);
}

Device::CircularBuffer::~CircularBuffer () {
  munmap (m_base, 2 * m_pagesize + 2 * m_buf_size);
}

/*
 * The amount to read can only grow unless someone calls read after this is
 * called.  No real good way to tie the two together.
 */
uint32_t Device::CircularBuffer::DataAvailable() {
  unsigned int amt;

  pthread_mutex_lock (&m_mutex);
  amt = m_written - m_read;	// item_size
  pthread_mutex_unlock (&m_mutex);

  return amt;
}


uint32_t Device::CircularBuffer::SpaceAvailable() {
  unsigned int amt;

  pthread_mutex_lock (&m_mutex);
  amt = m_buf_len - (m_written - m_read);
  pthread_mutex_unlock (&m_mutex);

  return amt;
}


#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif /* !MIN */

/*
 * m_buf_size is in terms of bytes
 * m_r and m_w are offsets in bytes
 * m_buf_len is in terms of m_item_size
 * buf_len is in terms of m_item_size
 * len, m_written, and m_read are all in terms of m_item_size
 */
uint32_t Device::CircularBuffer::Read(void *buf, const uint32_t buf_len) {
  unsigned int len;

  pthread_mutex_lock (&m_mutex);
  len = MIN (buf_len, m_written - m_read);
  memcpy(buf, reinterpret_cast<char *>(m_buf) + m_r, len * m_item_size);
  m_read += len;
  if (m_read == m_written) {
    m_r = m_w = 0;
    m_read = m_written = 0;
  } else {
    m_r = (m_r + len * m_item_size) % m_buf_size;
  }
  pthread_mutex_unlock (&m_mutex);

  return len;
}

/*
 * warning:
 *
 *	Don't use read() while you are peek()'ing.  write() should be
 *	okay unless you have an overwrite buffer.
 */
void *Device::CircularBuffer::Peek(uint32_t *buf_len) {
  uint32_t len;
  void *p;

  pthread_mutex_lock (&m_mutex);
  len = m_written - m_read;
  p = reinterpret_cast<char *>(m_buf) + m_r;
  pthread_mutex_unlock (&m_mutex);

  if (buf_len)
    *buf_len = len;

  return p;
}

void *Device::CircularBuffer::Poke(uint32_t *buf_len) {
  uint32_t len;
  void *p;

  pthread_mutex_lock (&m_mutex);
  len = m_buf_len - (m_written - m_read);
  p = reinterpret_cast<char *>(m_buf) + m_w;
  pthread_mutex_unlock (&m_mutex);

  if (buf_len)
    *buf_len = len;

  return p;
}


uint32_t Device::CircularBuffer::Purge(const uint32_t buf_len) {
  uint32_t len;

  pthread_mutex_lock (&m_mutex);
  len = MIN (buf_len, m_written - m_read);
  m_read += len;
  if (m_read == m_written) {
    m_r = m_w = 0;
    m_read = m_written = 0;
  } else {
    m_r = (m_r + len * m_item_size) % m_buf_size;
  }
  pthread_mutex_unlock (&m_mutex);

  return len;
}


uint32_t Device::CircularBuffer::Write(const void *buf,
                                       const uint32_t buf_len) {
  uint32_t len, buf_off = 0;

  pthread_mutex_lock (&m_mutex);
  if (m_overwrite) {
    if (buf_len > m_buf_len) {
      buf_off = buf_len - m_buf_len;
      len = m_buf_len;
    } else {
      len = buf_len;
    }
  } else {
    len = MIN (buf_len, m_buf_len - (m_written - m_read));
  }
  memcpy(reinterpret_cast<char *>(m_buf) + m_w,
         reinterpret_cast<const char *>(buf) + buf_off * m_item_size,
	  len * m_item_size);
  m_written += len;
  m_w = (m_w + len * m_item_size) % m_buf_size;
  if (m_written > m_buf_len + m_read) {
    m_read = m_written - m_buf_len;
    m_r = m_w;
  }
  pthread_mutex_unlock (&m_mutex);

  return len;
}


void Device::CircularBuffer::Wrote(uint32_t len) {
  pthread_mutex_lock (&m_mutex);
  m_written += len;
  m_w = (m_w + len * m_item_size) % m_buf_size;
  pthread_mutex_unlock (&m_mutex);
}


void Device::CircularBuffer::Flush() {
  pthread_mutex_lock (&m_mutex);
  m_read = m_written = 0;
  m_r = m_w = 0;
  pthread_mutex_unlock (&m_mutex);
}


void Device::CircularBuffer::FlushNoLock() {
  m_read = m_written = 0;
  m_r = m_w = 0;
}

void Device::CircularBuffer::Lock() {
  pthread_mutex_lock (&m_mutex);
}


void Device::CircularBuffer::Unlock () {
  pthread_mutex_unlock (&m_mutex);
}

uint32_t Device::CircularBuffer::BufferLength() {
  return m_buf_len;
}
