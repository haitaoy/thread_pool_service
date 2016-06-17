#ifndef THREAD_POOL_SERVICE_THREAD_WRAPPER_H
#define THREAD_POOL_SERVICE_THREAD_WRAPPER_H

#include <iostream>
#include <map>

struct ThreadWrapper {

  struct ThreadBase {
    virtual std::thread::id get_id() = 0;
    virtual bool joinable() = 0;
    virtual void join() = 0;
    virtual ~ThreadBase() { };
  };

  template<typename F>
  struct ThreadImpl: public ThreadBase {
    F tf_;
    std::thread t_;
    ThreadImpl(F &&tf) : tf_(std::move(tf)), t_(std::thread(tf_)) { }
    ThreadImpl(ThreadImpl &&other) : tf_(std::move(other.tf_)), t_(std::move(other.t_)) {
      other.t_ = nullptr;
    }

    std::thread::id get_id() {
      return t_.get_id();
    }

    bool joinable() {
      return t_.joinable();
    }

    void join() {
      t_.join();
    }
  };

  ThreadWrapper() = default;
  template<typename F>
  ThreadWrapper(F &f, bool enabled)
      : thread_(new ThreadImpl<F>(std::move(f))), enabled_(enabled), mutex_(new std::mutex),
        cond_(new std::condition_variable) {
  }
  ThreadWrapper(ThreadWrapper &&other)
      : enabled_(other.enabled_), thread_(std::move(other.thread_)), mutex_(std::move(other.mutex_)),
        cond_(std::move(other.cond_)) { }

  ThreadWrapper(const ThreadWrapper &) = default;
  ThreadWrapper &operator=(const ThreadWrapper &) = delete;

  void WaitForEnabling() {
    std::unique_lock<std::mutex> locked_guard(*mutex_);
    cond_->wait(locked_guard, [this] { return enabled_; });
  }

  std::thread::id get_id() {
    return thread_->get_id();
  }

  bool joinable() {
    return thread_->joinable();
  }

  void join() {
    thread_->join();
  }

  bool enabled_;
  std::unique_ptr<ThreadBase> thread_;
  std::unique_ptr<std::mutex> mutex_;
  std::unique_ptr<std::condition_variable> cond_;
};

typedef std::map<std::thread::id, std::shared_ptr<ThreadWrapper> > pool_type;

class ThreadsJoiner {
 public:
  explicit ThreadsJoiner(pool_type &threads) : threads_(threads) { }
  ~ThreadsJoiner() {
    for (auto &item : threads_) {
      auto thread = item.second;
      if (thread->joinable())
        thread->join();
    }
  }

 private:
  pool_type &threads_;
};

#endif //THREAD_POOL_SERVICE_THREAD_WRAPPER_H
