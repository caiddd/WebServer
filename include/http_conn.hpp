#ifndef HTTP_CONN_HPP_
#define HTTP_CONN_HPP_

#include "locker.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
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
  static constexpr int kReadBufferSize_ = 2048;
  static constexpr int kWriteBufferSize_ = 1024;

  // HTTP请求方法，这里只支持GET
  enum class METHOD : int {
    GET,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT
  };

  // 解析客户端请求时，主状态机的状态
  enum class CHECK_STATE : int {
    REQUESTLINE, // 当前正在分析请求行
    HEADER,      // 当前正在分析头部字段
    CONTENT      // 当前正在解析请求体
  };

  // 服务器处理HTTP请求的可能结果，报文解析的结果
  enum class HTTP_CODE : int {
    NO_REQUEST,        // 请求不完整，需要继续读取客户数据
    GET_REQUEST,       // 表示获得了一个完成的客户请求
    BAD_REQUEST,       // 表示客户请求语法错误
    NO_RESOURCE,       // 表示服务器没有资源
    FORBIDDEN_REQUEST, // 表示客户对资源没有足够的访问权限
    FILE_REQUEST,      // 文件请求,获取文件成功
    INTERNAL_ERROR,    // 表示服务器内部错误
    CLOSED_CONNECTION  // 表示客户端已经关闭连接了
  };

  // 从状态机的三种可能状态，即行的读取状态
  enum class LINE_STATUS : int {
    OK,  // 读取到一个完整的行
    BAD, // 行出错
    OPEN // 行数据尚且不完整
  };

  void Process();
  void Init(int sockfd, const sockaddr_in &kAddr);
  void Close();
  bool Read();
  bool Write();

 private:
  int sockfd_;
  int read_idx_;
  int start_line_;
  int checked_idx_;
  bool linger_; // HTTP请求是否要继续保持连接
  char *url_;   // 请求目标文件的文件名
  char *host_;
  char *version_; // 协议版本
  METHOD method_;
  sockaddr_in sock_addr_;
  CHECK_STATE check_state_;

  char read_buf_[kReadBufferSize_];

  void Init();
  HTTP_CODE ProcessRead();
  HTTP_CODE ParseRequestLine(char *text);
  HTTP_CODE ParseHeaders(char *text);
  HTTP_CODE ParseContents(char *text);
  HTTP_CODE DoRequest();
  LINE_STATUS ParseLine();
  char *GetLine() { return read_buf_ + start_line_; }
};

#endif // HTTP_CONN_HPP_