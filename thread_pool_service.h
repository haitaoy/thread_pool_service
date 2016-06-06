#ifndef THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H
#define THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H

#include <future>
#include "blocking_queue.h"
#include "task_wrapper.h"
#include "thread_wrapper.h"

/**
 * Thread pool service, simply implementing some features of ThreadPoolExecutor in java.util.concurrent,
 * but with some major differences:
 * 1. when constructing the service object, number of {#core_pool_size} threads are started.
 * 2. when the {#task_queue_} has be full and the evoke of {BlockingQueue.Push} returns false,
 *    number of {maximum_pool_size - core_pool_size} threads will be constructed, i.e, the number
 *    of threads in this thread pool service is increased to {#maximum_pool_size}.
 * 3. threads newly started are doing the same work with core threads, thus the tasks that failed to submitted
 *    will still not be executed, but this may pop task out of the queue and the tasks came latter may be successfully
 *    submitted to the queue.
 *  Also, the {#keep_alive_time} is the maximum idle time of the threads newly started.
 */
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
      std::shared_ptr<TaskWrapper> task;
      if(GetTask(task)) {
        if(task != nullptr)
          (*task)();
        else
          std::this_thread::yield();
      } else {
        //thread exit safely
      }
    }
  }

  void DoMoreWork(TaskWrapper&& task) {
    task();
    DoWork();
  }

  bool GetTask(std::shared_ptr<TaskWrapper>& task) {
    task = task_queue_.WaitAndPop(keep_alive_time_);
    if (task == nullptr && threads_.size() > core_pool_size_) //not atomic, continue later
      return false;
    else
      return true;
  }

  void Execute(TaskWrapper& task) {
    if(!task_queue_.Push(std::move(task))) {
      AddThread(task);
    }
  }

  void AddThread(TaskWrapper& task) {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    if(threads_.size() < maximum_pool_size_)
      threads_.push_back(std::thread(&ThreadPoolService::DoMoreWork, this, std::move(task)));
  }

  bool finished;
  int core_pool_size_;
  int maximum_pool_size_;
  int keep_alive_time_;
  BlockingQueue<TaskWrapper> &task_queue_;
  std::vector<std::thread> threads_;
  ThreadsJoiner joiner;

  std::mutex mutex_;
};

#endif //THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H
