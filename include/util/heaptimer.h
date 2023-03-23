#ifndef UTIL_HEAPTIMER_H
#define UTIL_HEAPTIMER_H

#include <arpa/inet.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <functional>
#include <queue>
#include <unordered_map>

#include "log.h"

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

struct TimerNode {
  int id;
  TimeStamp expires;
  TimeoutCallBack cb;
  bool operator<(const TimerNode& t) const { return expires < t.expires; }
};

class HeapTimer {
 public:
  HeapTimer() { heap_.reserve(64); }

  ~HeapTimer() { clear(); }

  void adjust(int id, int timeout);

  void add(int id, int timeOut, const TimeoutCallBack& cb);

  void doWork(int id);

  void clear();

  void tick();

  void pop();

  size_t GetNextTick();

 private:
  void del_(size_t i);

  void siftup_(size_t i);

  bool siftdown_(size_t index, size_t n);

  void SwapNode_(size_t i, size_t j);

  std::vector<TimerNode> heap_;

  std::unordered_map<int, size_t> ref_;
};

#endif // UTIL_HEAPTIMER_H