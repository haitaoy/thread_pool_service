#include <iostream>
#include "../thread_pool_service.h"

struct Worker {
  void operator()(){
    std::cout << "do work in struct\n";
  }
};

void work() {
  std::cout << "do work in function\n";
}

int main() {
  BlockingQueue<TaskWrapper> queue(10);

  Worker worker;
  std::shared_ptr<ThreadPoolService> service(new ThreadPoolService(2, 2, 10, queue));

  service->submit(worker);
  service->submit(work);

  return 0;
}