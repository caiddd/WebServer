#ifndef UTIL_BUFFER_H
#define UTIL_BUFFER_H

#include <sys/uio.h> //readv
#include <unistd.h>  // write

#include <atomic>
#include <cassert>
#include <cstring> //perror
#include <iostream>
#include <vector> //readv

class Buffer {
 public:
  explicit Buffer(int initBuffSize = 1024);
  ~Buffer() = default;

  [[nodiscard]] size_t WritableBytes() const;
  [[nodiscard]] size_t ReadableBytes() const;
  [[nodiscard]] size_t PrependableBytes() const;

  [[nodiscard]] const char* Peek() const;
  void EnsureWriteable(size_t len);
  void HasWritten(size_t len);

  void Retrieve(size_t len);
  void RetrieveUntil(const char* end);

  void RetrieveAll();
  std::string RetrieveAllToStr();

  [[nodiscard]] const char* BeginWriteConst() const;
  char* BeginWrite();

  void Append(const std::string& str);
  void Append(const char* str, size_t len);
  void Append(const void* data, size_t len);
  void Append(const Buffer& buff);

  ssize_t ReadFd(int fd, int* Errno);
  ssize_t WriteFd(int fd, int* Errno);

 private:
  char* BeginPtr_();
  [[nodiscard]] const char* BeginPtr_() const;
  void MakeSpace_(size_t len);

  std::vector<char> buffer_;
  std::atomic<std::size_t> readPos_;
  std::atomic<std::size_t> writePos_;
};

#endif // UTIL_BUFFER_H