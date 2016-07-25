#ifndef THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H
#define THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H

#include <future>
#include "blocking_queue.h"
#include "task_wrapper.h"
#include "thread_wrapper.h"

/**
 * Thread pool service, simply implementing some features of ThreadPoolExecutor in java.util.concurrent,
 * but with some major differences:
 * 1. when constructing the service object, number of {#maximum_pool_size_} threads are started, but only number
 *    of {#core_pool_size_} threads will do the real work(enabled), the additive thread are disabled and waiting.
 * 2. when the {#task_queue_} has be full and the evoke of {BlockingQueue.Push} returns false,
 *    the {maximum_pool_size - core_pool_size} of disabled threads will be enabled one by one.
 * 3. threads newly started are doing the same work with core threads, thus the tasks that failed to submitted
 *    will still not be executed, but this may pop tasks out of the queue and the tasks came latter may be successfully
 *    submitted to the queue.
 *  Also, the {#keep_alive_time} is the maximum idle time of the threads newly started.
 */
class ThreadPoolService {

  typedef std::map<std::thread::id, std::shared_ptr<ThreadWrapper> >::value_type thread_type;
 public:
  ThreadPoolService(int core_pool_size,
                    int maximum_pool_size,
                    int keep_alive_time,
                    BlockingQueue<TaskWrapper> &task_queue)
      : finished_(false),
        core_pool_size_(core_pool_size),
        maximum_pool_size_(maximum_pool_size),
        keep_alive_time_(keep_alive_time),
        task_queue_(task_queue),
        joiner_working_(working_pool_),
        joiner_waiting_(waiting_pool_), barrier_(maximum_pool_size_ + 1) {

    for (int i = 0; i < core_pool_size_; ++i) {
      AddWorker();
    }
    for (int i = core_pool_size_; i < maximum_pool_size_; ++i) {
      AddWaiter();
    }

    finished_ = false;
    barrier_.wait();
    std::this_thread::yield();
  }

  /**
   * submit a task to the thread pool service, return a future to get the result later.
   */
  template<typename F>
  std::future<typename std::result_of<F()>::type> Submit(F f) {
    typedef typename std::result_of<F()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> future(task.get_future());

    if (Execute(TaskWrapper(std::move(task))))
      return future;
    else
      return (std::future<result_type>());
  }

  /**
   * suspend the thread pool now, submits will be failed and the worker thread will still do the task until the queue is
   * empty.
   */
  void Shutdown() {
    finished_ = true;
    barrier_.wait();
  }

 private:
  void DoWork() {

//    while (!CanWork(working_pool_))
//      std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier_.wait();

    std::shared_ptr<ThreadWrapper> current_worker = working_pool_[std::this_thread::get_id()];
    while (current_worker->enabled_) {
      TaskWrapper task;
      if (task_queue_.Pop(task))
        task();
      else if (finished_)
        current_worker->enabled_ = false;
      else
        std::this_thread::yield();
    }

    barrier_.wait();
  }

  void WaitAndWork() {

//    while (!CanWork(waiting_pool_))
//      std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier_.wait();

    std::shared_ptr<ThreadWrapper> current_worker = waiting_pool_[std::this_thread::get_id()];
    current_worker->WaitToWork();

    while (current_worker->enabled_) {
      std::shared_ptr<TaskWrapper> task;
      if (finished_)
        current_worker->enabled_ = false;
      else if (GetTask(task)) {
        if (task != nullptr)
          (*task)();
        else
          std::this_thread::yield();
      } else {
        current_worker->StopWorking();
        {
          std::lock_guard<std::mutex> lg(mutex_);
          waiting_pool_.insert(thread_type(std::this_thread::get_id(), current_worker));
          working_pool_.erase(std::this_thread::get_id());
        }
        current_worker->WaitToWork();
      }
    }

    barrier_.wait();
  }

  bool GetTask(std::shared_ptr<TaskWrapper> &task) {
    task = task_queue_.WaitAndPop(keep_alive_time_);
    if (task == nullptr && ThreadPoolSize() > core_pool_size_)
      return false;
    else
      return true;
  }

  bool Execute(TaskWrapper &&task) {
    if (finished_)
      return false;

    if (!task_queue_.Push(std::move(task))) {
      AddThread();
      return false;
    }
    return true;
  }

  void AddThread() {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    if (working_pool_.size() < maximum_pool_size_ && waiting_pool_.size() > 0) {
      std::shared_ptr<ThreadWrapper> &waken_worker = waiting_pool_.begin()->second;
      working_pool_.insert(*(waiting_pool_.begin()));
      waiting_pool_.erase( waiting_pool_.begin());
      waken_worker->StartToWork();
    }
  }

  void AddWorker() {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    auto boot = std::bind(&ThreadPoolService::DoWork, this);
    std::shared_ptr<ThreadWrapper> worker(new ThreadWrapper(boot, true, true));
    working_pool_.insert(thread_type(worker->get_id(), worker));
  }

  void AddWaiter() {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    auto boot = std::bind(&ThreadPoolService::WaitAndWork, this);
    std::shared_ptr<ThreadWrapper> worker(new ThreadWrapper(boot, true, false));
    waiting_pool_.insert(thread_type(worker->get_id(), worker));
  }

  void NotifyToExit() {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    if (waiting_pool_.empty())
      return;

  }

  bool CanWork(pool_type &pool) {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    return pool.find(std::this_thread::get_id()) != pool.end();
  }

  int ThreadPoolSize() {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    return working_pool_.size() + waiting_pool_.size();
  }

  std::atomic<bool> finished_;
  int core_pool_size_;
  int maximum_pool_size_;
  int keep_alive_time_;
  BlockingQueue<TaskWrapper> &task_queue_;
  pool_type working_pool_;
  pool_type waiting_pool_;
  ThreadsJoiner joiner_working_;
  ThreadsJoiner joiner_waiting_;
  ThreadBarrier barrier_;

  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif //THREAD_POOL_SERVICE_THREAD_POOL_SERVICE_H
