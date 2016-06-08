#include <iostream>
#include "../thread_pool_service.h"

static std::atomic<int> done_num(1);

struct Worker {
  void operator()(){
    std::cout << "do work in struct, " << done_num.fetch_add(1) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
};

void work() {
  std::cout << "do work in function, " << done_num.fetch_add(1) << std::endl;
}

int main() {
  BlockingQueue<TaskWrapper> queue(1000);

  Worker worker;
  std::shared_ptr<ThreadPoolService> service(new ThreadPoolService(10, 10, 10, queue));

  for (int i = 0; i < 2000; ++i) {
    service->submit(worker);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return 0;
}