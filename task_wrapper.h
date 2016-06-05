#ifndef THREAD_POOL_SERVICE_TASK_WRAPPER_H
#define THREAD_POOL_SERVICE_TASK_WRAPPER_H

#include <memory>

class TaskWrapper {

  struct CallerBase {
    virtual void call() = 0;
    virtual ~CallerBase() { }
  };

  template<typename F>
  struct Caller: public CallerBase {
    F cf_;
    Caller(F &&cf) : cf_(std::move(cf)) { }
    void call() { cf_(); }
  };

 public:
  template<typename F>
  TaskWrapper(F &&f) : caller(new Caller<F>(std::move(f))) { }
  TaskWrapper(TaskWrapper&& other) : caller(std::move(other.caller)) { }
  TaskWrapper& operator=(TaskWrapper&& other) {
    if(this != &other) {
      caller = std::move(other.caller);
    }
    return *this;
  }
  void operator()() { caller->call(); };

  TaskWrapper(const TaskWrapper &) = delete;
  TaskWrapper &operator=(const TaskWrapper &) = delete;

 private:
  std::unique_ptr<CallerBase> caller;
};

#endif //THREAD_POOL_SERVICE_TASK_WRAPPER_H
