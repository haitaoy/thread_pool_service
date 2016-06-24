#include <iostream>
#include "thread_pool_service.h"

static std::atomic<int> done_num(1);

struct worker {
  void operator()(){
    std::cout << std::this_thread::get_id() << " do work in struct, " << done_num.fetch_add(1) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
};

int main() {
  BlockingQueue<TaskWrapper> queue(1000);

  std::shared_ptr<ThreadPoolService> service(new ThreadPoolService(10, 10, 10, queue));

  for (int i = 0; i < 2000; ++i) {
    worker worker;
    service->Submit(worker);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (i == 1000)
      service->Shutdown();
  }

  return 0;
}