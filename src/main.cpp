/*
TODO : 使用智能指针管理内存
*/
#include "thread_pool.hpp"
#include "data.hpp"
#include "http_conn.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int port;
int epollfd;
int listenfd;
HttpConn *users;
ThreadPool<HttpConn> *pool;
epoll_event events[MAX_EVENT_NUMBER];

void AddSig(int sig, void(handler)(int)) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handler;
  sigfillset(&sa.sa_mask);
  sigaction(sig, &sa, nullptr);
}

void CreateThreadPool() {
  printf("线程池构建中\n");
  try {
    pool = new ThreadPool<HttpConn>;
  } catch (...) {
    printf("线程池构建错误！\n");
    exit(-1);
  }
  users = new HttpConn[MAX_FD];
}

void CreateServer() {
  listenfd = socket(PF_INET, SOCK_STREAM, 0);
  int reuse = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  sockaddr_in address;
  address.sin_family = PF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  bind(listenfd, (sockaddr *)&address, sizeof(address));
  listen(listenfd, 5);
}

void CreateEpoll() {
  epollfd = epoll_create(1);
  AddFd(epollfd, listenfd, false);
  HttpConn::epollfd_ = epollfd;
}

void ClientConnect() {
  struct sockaddr_in client_address;
  socklen_t client_addrlen = sizeof(client_address);
  int const connfd =
      accept(listenfd, (sockaddr *)&client_address, &client_addrlen);
  if (HttpConn::user_count_ >= MAX_FD) {
    // TODO : 给客户端发送信息表示服务器正忙
    close(connfd);
    return;
  }
  users[connfd].Init(connfd, client_address);
}

void ClientEventDeal(int i) {
  // TODO : 之后解释这几个if判断的情况是什么
  int const sockfd = events[i].data.fd;
  if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
    users[sockfd].Close();
  } else if (events[i].events & EPOLLIN) {
    if (users[sockfd].Read()) {
      pool->Append(users + sockfd);
    } else {
      users[sockfd].Close();
    }
  } else if (events[i].events & EPOLLOUT) {
    if (!users[sockfd].Write()) { users[sockfd].Close(); }
  }
}

void RunServer() {
  while (true) {
    int const num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if (num < 0 and errno != EINTR) {
      printf("epoll failure\n");
      break;
    }
    for (int i = 0; i < num; ++i) {
      if (events[i].data.fd == listenfd) {
        ClientConnect();
      } else {
        ClientEventDeal(i);
      }
    }
  }
}

void CloseServer() {
  close(epollfd);
  close(listenfd);
  delete[] users;
  delete pool;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("按照如下格式运行 : %s port_number", basename(argv[0]));
    exit(-1);
  }

  port = atoi(argv[1]);
  AddSig(SIGPIPE, SIG_IGN);
  CreateThreadPool();
  CreateServer();
  CreateEpoll();
  RunServer();
  CloseServer();
  return 0;
}