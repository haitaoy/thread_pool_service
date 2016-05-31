#ifndef THREAD_POOL_SERVICE_BLOCKING_QUEUE_H
#define THREAD_POOL_SERVICE_BLOCKING_QUEUE_H

#include <mutex>
#include <memory>
#include <condition_variable>
#include <queue>

template<typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(int size) : size_(size) { }

  bool Push(T value) {
    std::shared_ptr<T> new_item(std::make_shared<T>(std::move(value)));

    std::lock_guard<std::mutex> locked_guard(mutex_);
    if (queue_.size() >= size_)
      return false;

    queue_.push(new_item);
    cond_.notify_one();

    return true;
  }

  std::shared_ptr<T> WaitAndPop() {
    std::unique_lock<std::mutex> locked_guard(mutex_);
    cond_.wait(locked_guard, [this] { return !queue_.empty(); });

    std::shared_ptr<T> item = queue_.front();
    queue_.pop();
    return item;
  }

 private:
  std::mutex mutex_;
  std::queue<std::shared_ptr<T> > queue_;
  int size_;
  std::condition_variable cond_;
};

#endif //THREAD_POOL_SERVICE_BLOCKING_QUEUE_H
