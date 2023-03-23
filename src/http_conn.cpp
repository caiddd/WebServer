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
  // event.events = EPOLLIN | EPOLLRDHUP;
  event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
  if (oneshot) { event.events |= EPOLLONESHOT; }
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  SetNonBlocking(fd);
}

void HttpConn::Init(int sockfd, const sockaddr_in &kAddr) {
  sockfd_ = sockfd;
  sock_addr_ = kAddr;
  int reuse = 1;
  setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  AddFd(epollfd_, sockfd_, true);
  user_count_++;

  Init();
}

void HttpConn::Init() {
  checked_idx_ = 0;
  start_line_ = 0;
  read_idx_ = 0;
  url_ = nullptr;
  host_ = nullptr;
  version_ = nullptr;
  linger_ = false;
  method_ = METHOD::GET;
  check_state_ = CHECK_STATE::REQUESTLINE;
  memset(read_buf_, 0, kReadBufferSize_);
}

void HttpConn::Close() {
  if (sockfd_ != -1) {
    RemoveFd(epollfd_, sockfd_);
    sockfd_ = -1;
    user_count_--;
  }
}

bool HttpConn::Read() {
  if (read_idx_ >= kReadBufferSize_) { return false; }
  int bytes_read = 0;
  while (true) {
    bytes_read =
        recv(sockfd_, read_buf_ + read_idx_, kReadBufferSize_ - read_idx_, 0);
    if (bytes_read == -1) {
      if (errno == EAGAIN or errno == EWOULDBLOCK) { break; }
      return false;
    }
    if (bytes_read == 0) { return false; }
    read_idx_ += bytes_read;
  }
  // TODO : 删除调试语句
  printf("读取到数据: %s\n", read_buf_);
  return true;
}

void HttpConn::Process() {
  HTTP_CODE const kread_ret = ProcessRead();
  if (kread_ret == HTTP_CODE::NO_REQUEST) {
    Modfd(epollfd_, sockfd_, EPOLLIN);
    return;
  }
  // TODO : 生成响应
  printf("testProcess\n");
}

HttpConn::HTTP_CODE HttpConn::ProcessRead() {
  LINE_STATUS line_status = LINE_STATUS::OK;
  HTTP_CODE ret = HTTP_CODE::NO_REQUEST;
  char *text = nullptr;
  while (
      (check_state_ == CHECK_STATE::CONTENT and line_status == LINE_STATUS::OK)
      or (line_status = ParseLine()) == LINE_STATUS::OK) {
    text = GetLine();
    start_line_ = checked_idx_;
    printf("Got 1 Http Line%s\n", text);
    switch (check_state_) {
      case CHECK_STATE::REQUESTLINE: {
        ret = ParseRequestLine(text);
        if (ret == HTTP_CODE::BAD_REQUEST) { return ret; }
        break;
      }

      case CHECK_STATE::HEADER: {
        ret = ParseHeaders(text);
        if (ret == HTTP_CODE::BAD_REQUEST) { return ret; }
        if (ret == HTTP_CODE::GET_REQUEST) { return DoRequest(); }
        break;
      }

      case CHECK_STATE::CONTENT: {
        ret = ParseContents(text);
        if (ret == HTTP_CODE::GET_REQUEST) { return DoRequest(); }
        line_status = LINE_STATUS::OPEN;
        break;
      }

      default:
        return HTTP_CODE::INTERNAL_ERROR;
    }
  }
  return HTTP_CODE::NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::ParseRequestLine(char *text) {
  url_ = strpbrk(text, " \t");
  *url_++ = '\0';
  char *method = text;
  // TODO : 后续请求方法还需在此处完善
  if (strcasecmp(method, "GET") != 0) { return HTTP_CODE::BAD_REQUEST; }
  method_ = METHOD::GET;
  version_ = strpbrk(url_, " \t");
  if (!version_) { return HTTP_CODE::BAD_REQUEST; }
  *version_++ = '\0';
  if (strcasecmp(version_, "HTTP/1.1") != 0) { return HTTP_CODE::BAD_REQUEST; }
  if (strncasecmp(url_, "http://", 7) == 0) {
    url_ += 7;
    url_ = strchr(url_, '/');
  }
  if (!url_ or url_[0] != '/') { return HTTP_CODE::BAD_REQUEST; }
  check_state_ = CHECK_STATE::HEADER;
  return HTTP_CODE::NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::ParseHeaders(char *text) {
  return HTTP_CODE::NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::ParseContents(char *text) {
  return HTTP_CODE::NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::DoRequest() {
  return HTTP_CODE::NO_REQUEST;
}

HttpConn::LINE_STATUS HttpConn::ParseLine() {
  char tmp;
  for (; checked_idx_ < read_idx_; ++checked_idx_) {
    tmp = read_buf_[checked_idx_];
    if (tmp == '\r') {
      if (checked_idx_ + 1 == read_idx_) { return LINE_STATUS::OPEN; }
      if (read_buf_[checked_idx_ + 1] == '\n') {
        read_buf_[checked_idx_++] = 0;
        read_buf_[checked_idx_++] = 0;
        return LINE_STATUS::OK;
      }
      return LINE_STATUS::BAD;
    }
    if (tmp == '\n') {
      if (checked_idx_ > 1 and read_buf_[checked_idx_ - 1] == '\r') {
        read_buf_[checked_idx_ - 1] = 0;
        read_buf_[checked_idx_++] = 0;
        return LINE_STATUS::OK;
      }
      return LINE_STATUS::BAD;
    }
  }
  return LINE_STATUS::OPEN;
}

bool HttpConn::Write() {
  // TODO : 一次性写完数据
  printf("testWrite\n");
  return true;
}