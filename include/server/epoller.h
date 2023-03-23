#ifndef SERVER_EPOLLER_H
#define SERVER_EPOLLER_H

#include <fcntl.h>     // fcntl()
#include <sys/epoll.h> //epoll_ctl()
#include <unistd.h>    // close()

#include <cassert> // close()
#include <cerrno>
#include <vector>

class Epoller {
 public:
  explicit Epoller(int maxEvent = 1024);

  ~Epoller();

  bool AddFd(int fd, uint32_t events) const;

  bool ModFd(int fd, uint32_t events) const;

  bool DelFd(int fd) const;

  int Wait(int timeoutMs = -1);

  int GetEventFd(size_t i) const;

  uint32_t GetEvents(size_t i) const;

 private:
  int epollFd_;

  std::vector<struct epoll_event> events_;
};

#endif // SERVER_EPOLLER_H