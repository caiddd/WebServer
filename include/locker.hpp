#ifndef LOCKER_HPP_
#define LOCKER_HPP_

#include <pthread.h>
#include <semaphore.h>
#include <exception>

class Mutex {
 public:
  Mutex() {
    if(pthread_mutex_init(&m_mutex_, NULL) != 0) {
      throw std::exception();
    }
  }
  ~Mutex() {
    pthread_mutex_destroy(&m_mutex_);
  }
  bool Lock() {
    return pthread_mutex_lock(&m_mutex_) == 0;
  }
  bool Unlock() {
    return pthread_mutex_unlock(&m_mutex_) == 0;
  }
  pthread_mutex_t * m_mutex() {
    return &m_mutex_;
  }

 private:
  pthread_mutex_t m_mutex_;
};

class Cond {
 public:
  Cond() {
    if (pthread_cond_init(&m_cond_, NULL) != 0) {
      throw std::exception();
    }
  }
  ~Cond() {
    pthread_cond_destroy(&m_cond_);
  }
  bool Wait(pthread_mutex_t *mutex) {
    return pthread_cond_wait(&m_cond_, mutex) == 0;
  }
  bool TimeWait(pthread_mutex_t *mutex, timespec *time) {
    return pthread_cond_timedwait(&m_cond_, mutex, time) == 0;
  }
  bool Signal() {
    return pthread_cond_signal(&m_cond_) == 0;
  }
  bool Broadcast() {
    return pthread_cond_broadcast(&m_cond_) == 0;
  }

 private:
  pthread_cond_t m_cond_;
};

class Sem {
 public:
  Sem() {
    if (sem_init(&m_sem_, 0, 0) != 0) {
      throw std::exception();
    }
  }
  Sem(int num) {
    if (sem_init(&m_sem_, 0, num) != 0) {
      throw std::exception();
    }
  }
  ~Sem() {
    sem_destroy(&m_sem_);
  }
  bool Wait() {
    return sem_wait(&m_sem_) == 0;
  }
  bool Post() {
    return sem_post(&m_sem_) == 0;
  }

 private:
  sem_t m_sem_;
};

#endif  // LOCKER_HPP_