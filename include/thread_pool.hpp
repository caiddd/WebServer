#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include "locker.hpp"
#include <cstdio>
#include <list>

template<typename Task>
class ThreadPool {
 public:
  explicit ThreadPool(int thread_num = 8, int max_request = 10000);
  ~ThreadPool();
  bool Append(Task *request);

 private:
  bool stop_{false};
  int thread_num_;
  int max_request_;

  Sem queue_stat_;
  Mutex queue_locker_;

  pthread_t *threads_{nullptr};
  std::list<Task *> work_queue_;

  void run();
  static void *Worker(void *arg);
};

template<typename Task>
ThreadPool<Task>::ThreadPool(int thread_num, int max_request)
    : thread_num_(thread_num), max_request_(max_request) {
  if (thread_num_ <= 0 or max_request_ <= 0) { throw std::exception(); }
  threads_ = new pthread_t[thread_num_];
  if (!threads_) { throw std::exception(); }
  for (int i = 0; i < thread_num_; ++i) {
    printf("create the %dth thread\n", i);
    if (pthread_create(threads_ + i, NULL, Worker, this) != 0) {
      delete[] threads_;
      throw std::exception();
    }
    if (pthread_detach(threads_[i])) {
      delete[] threads_;
      throw std::exception();
    }
  }
}

template<typename Task>
ThreadPool<Task>::~ThreadPool() {
  delete[] threads_;
  stop_ = true;
}

template<typename Task>
bool ThreadPool<Task>::Append(Task *request) {
  queue_locker_.Lock();
  if (work_queue_.size() > max_request_) {
    queue_locker_.Unlock();
    return false;
  }
  work_queue_.emplace_back(request);
  queue_locker_.Unlock();
  queue_stat_.Post();
  return true;
}

template<typename Task>
void *ThreadPool<Task>::Worker(void *arg) {
  auto pool = static_cast<ThreadPool *>(arg);
  pool->run();
  return pool;
}

template<typename Task>
void ThreadPool<Task>::run() {
  while (stop_) {
    queue_stat_.Wait();
    queue_locker_.Lock();
    if (work_queue_.empty()) {
      queue_locker_.Unlock();
      continue;
    }
    auto request = work_queue_.front();
    work_queue_.pop_front();
    queue_locker_.Unlock();
    if (!request) { continue; }
    request->process();
  }
}

#endif // THREAD_POOL_HPP_
