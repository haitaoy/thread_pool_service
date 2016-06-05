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
    empty_cond_.notify_one();
    return true;
  }

  void WaitAndPush(T value) {
    std::shared_ptr<T> new_item(std::make_shared<T>(std::move(value)));
    std::unique_lock<std::mutex> locked_guard(mutex_);
    full_cond_.wait(locked_guard, [this] { return queue_.size() >= size_; });

    queue_.push(new_item);
    empty_cond_.notify_one();
  }

  bool Pop(T &value) {
    std::lock_guard<std::mutex> locked_guard(mutex_);
    if (queue_.empty())
      return false;

    value = std::move(*queue_.front());
    queue_.pop();
    full_cond_.notify_one();
    return true;
  }

  std::shared_ptr<T> WaitAndPop() {
    std::unique_lock<std::mutex> locked_guard(mutex_);
    empty_cond_.wait(locked_guard, [this] { return !queue_.empty(); });

    std::shared_ptr<T> item = queue_.front();
    queue_.pop();
    full_cond_.notify_one();
    return item;
  }

 private:
  std::mutex mutex_;
  std::queue<std::shared_ptr<T> > queue_;
  int size_;
  std::condition_variable empty_cond_, full_cond_;
};

#endif //THREAD_POOL_SERVICE_BLOCKING_QUEUE_H
