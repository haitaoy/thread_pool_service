#ifndef THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H
#define THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H

#include <future>
#include "blocking_queue.h"
#include "task_wrapper.h"
#include "thread_wrapper.h"

class ThreadPoolService {
 public:
  ThreadPoolService(int core_pool_size,
                    int maximum_pool_size,
                    int keep_alive_time,
                    BlockingQueue<TaskWrapper> &task_queue)
      : finished(false),
        core_pool_size_(core_pool_size),
        maximum_pool_size_(maximum_pool_size),
        keep_alive_time_(keep_alive_time),
        task_queue_(task_queue),
        joiner(threads_) {
    for (int i = 0; i < core_pool_size_; ++i) {
      threads_.push_back(std::thread(&ThreadPoolService::DoWork, this));
    }
  }

  template<typename F>
  std::future<typename std::result_of<F()>::type> submit(F f) {
    typedef typename std::result_of<F()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> future(task.get_future());
    task_queue_.WaitAndPush(std::move(task));
    return future;
  }

 private:
  void DoWork() {
    while (!finished) {
      TaskWrapper task;
      if (task_queue_.Pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }

  bool finished;
  int core_pool_size_;
  int maximum_pool_size_;
  int keep_alive_time_;
  std::vector<std::thread> threads_;
  ThreadsJoiner joiner;
  BlockingQueue<TaskWrapper> &task_queue_;
};

#endif //THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H
