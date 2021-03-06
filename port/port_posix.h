// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// See port_example.h for documentation for the following types/functions.

#ifndef STORAGE_LEVELDB_PORT_PORT_POSIX_H_
#define STORAGE_LEVELDB_PORT_PORT_POSIX_H_

// port/port_config.h availability is automatically detected via __has_include
// in newer compilers. If LEVELDB_HAS_CONFIG_H is defined, it overrides the
// configuration detection in older compilers.
#if defined(LEVELDB_HAS_PORT_CONFIG_H)
#include "port/port_config.h"
#endif  // defined(LEVELDB_HAS_PORT_CONFIG_H)
#if defined(__has_include)
#if __has_include("port/port_config.h")
#include "port/port_config.h"
#endif  // __has_include("port/port_config.h")
#endif  // defined(__has_include)

#include <pthread.h>
#if defined(HAVE_CRC32C)
#include <crc32c/crc32c.h>
#endif  // defined(HAVE_CRC32C)
#ifdef HAVE_SNAPPY
#include <snappy.h>
#endif  // defined(HAVE_SNAPPY)
#include <stdint.h>
#include <string>
#include "port/atomic_pointer.h"

// Use fread/fwrite/fflush on platforms without _unlocked variants
#if !HAVE_FREAD_UNLOCKED
#define fread_unlocked fread
#endif  // !HAVE_FREAD_UNLOCKED
#if !HAVE_FWRITE_UNLOCKED
#define fwrite_unlocked fwrite
#endif  // !HAVE_FWRITE_UNLOCKED
#if !HAVE_FLUSH_UNLOCKED
#define fflush_unlocked fflush
#endif  // !HAVE_FLUSH_UNLOCKED

#if !HAVE_FDATASYNC
#define fdatasync fsync
#endif  // !HAVE_FDATASYNC

namespace leveldb {
namespace port {

static const bool kLittleEndian = !LEVELDB_IS_BIG_ENDIAN;

class CondVar;

class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();
  void AssertHeld() { }

 private:
  friend class CondVar;
  pthread_mutex_t mu_;

  // No copying
  Mutex(const Mutex&);
  void operator=(const Mutex&);
};

class CondVar {
 public:
  explicit CondVar(Mutex* mu);
  ~CondVar();
  void Wait();
  void Signal();
  void SignalAll();
 private:
  pthread_cond_t cv_;
  Mutex* mu_;
};

typedef pthread_once_t OnceType;
#define LEVELDB_ONCE_INIT PTHREAD_ONCE_INIT
extern void InitOnce(OnceType* once, void (*initializer)());

inline bool Snappy_Compress(const char* input, size_t length,
                            ::std::string* output) {
#ifdef HAVE_SNAPPY
  output->resize(snappy::MaxCompressedLength(length));
  size_t outlen;
  snappy::RawCompress(input, length, &(*output)[0], &outlen);
  output->resize(outlen);
  return true;
#endif  // defined(HAVE_SNAPPY)

  return false;
}

inline bool Snappy_GetUncompressedLength(const char* input, size_t length,
                                         size_t* result) {
#ifdef HAVE_SNAPPY
  return snappy::GetUncompressedLength(input, length, result);
#else
  return false;
#endif  // defined(HAVE_SNAPPY)
}

inline bool Snappy_Uncompress(const char* input, size_t length,
                              char* output) {
#ifdef HAVE_SNAPPY
  return snappy::RawUncompress(input, length, output);
#else
  return false;
#endif  // defined(HAVE_SNAPPY)
}

inline bool GetHeapProfile(void (*func)(void*, const char*, int), void* arg) {
  return false;
}

inline uint32_t AcceleratedCRC32C(uint32_t crc, const char* buf, size_t size) {
#if defined(HAVE_CRC32C)
  return ::crc32c::Extend(crc, reinterpret_cast<const uint8_t*>(buf), size);
#else
  return 0;
#endif  // defined(HAVE_CRC32C)
}

}  // namespace port
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_PORT_PORT_POSIX_H_
