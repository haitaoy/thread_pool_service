#include <iostream>
#include "thread_pool_service.h"
#include "worker.h"

int main(int argc, char *argv[]) {
  if (argc != 7) {
    std::cout << "usage: thread_pool_service [core-num] [max-num] [task-num] [alive-time] [wait_size] [freq]"
        << std::endl;
    return -1;
  }

  int core_num = atoi(argv[1]);
  int max_num = atoi(argv[2]);
  int task_num = atoi(argv[3]);
  int alive_time = atoi(argv[4]);
  int wait_size = atoi(argv[5]);
  int freq = atoi(argv[6]);

  BlockingQueue<TaskWrapper> queue(wait_size);
  ThreadPoolService service(core_num, max_num, alive_time, queue);

  int submit_wait = 1000 / freq;
  for (int i = 0; i < task_num; ++i) {
    Worker worker;
    service.Submit(worker);

    std::this_thread::sleep_for(std::chrono::milliseconds(submit_wait));
  }

  service.Shutdown();

  return 0;
}