
#ifndef THREAD_POOL_SERVICE_THREAD_WRAPPER_H
#define THREAD_POOL_SERVICE_THREAD_WRAPPER_H

class ThreadsJoiner {
 public:
  explicit ThreadsJoiner(std::vector<std::thread> &threads) : threads_(threads) { }
  ~ThreadsJoiner() {
    for (auto& item : threads_) {
      if (item.joinable())
        item.join();
    }
  }

 private:
  std::vector<std::thread> &threads_;
};

#endif //THREAD_POOL_SERVICE_THREAD_WRAPPER_H
