#ifndef HTTP_CONN_HPP_
#define HTTP_CONN_HPP_

#include "locker.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

void RemoveFd(int epollfd, int fd);
void Modfd(int epollfd, int fd, int ev);
void AddFd(int epollfd, int fd, bool oneshot);

class HttpConn {
 public:
  static int epollfd_;
  static int user_count_;
  void Process();
  void Init(int sockfd, const sockaddr_in &addr);
  void Close();
  bool Read();
  bool Write();

 private:
  int sockfd_;
  sockaddr_in sock_addr_;
};

#endif // HTTP_CONN_HPP_