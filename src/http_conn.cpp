#include "http_conn.hpp"

int HttpConn::epollfd_ = -1;
int HttpConn::user_count_ = 0;

void SetNonBlocking(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void RemoveFd(int epollfd, int fd) {
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
  close(fd);
}

void Modfd(int epollfd, int fd, int ev) {
  epoll_event event;
  event.data.fd = fd;
  event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void AddFd(int epollfd, int fd, bool oneshot) {
  epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLRDHUP;
  if (oneshot) { event.events |= EPOLLONESHOT; }
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  SetNonBlocking(fd);
}

void HttpConn::Init(int sockfd, const sockaddr_in &addr) {
  sockfd_ = sockfd;
  sock_addr_ = addr;
  int reuse = 1;
  setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  AddFd(epollfd_, sockfd_, true);
  user_count_++;
}

void HttpConn::Close() {
  if (sockfd_ != -1) {
    RemoveFd(epollfd_, sockfd_);
    sockfd_ = -1;
    user_count_--;
  }
}

bool HttpConn::Read() {
  // TODO : 一次性读完数据
  printf("testRead\n");
  return true;
}

bool HttpConn::Write() {
  // TODO : 一次性写完数据
  printf("testWrite\n");
  return true;
}

void HttpConn::Process() {
  // TODO : 解析HTTP请求，生成响应
  printf("testProcess\n");
}