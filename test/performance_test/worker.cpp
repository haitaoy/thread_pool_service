#include <cstdlib>
#include <thread>
#include <iostream>
#include <atomic>
#include "worker.h"

static std::atomic<int> done_num(1);

void Worker::operator()() {
  std::cout << std::this_thread::get_id() << " do work in struct: " << done_num.fetch_add(1) << std::endl;
  int working_time = rand() % 1000 + 1;
  std::this_thread::sleep_for(std::chrono::milliseconds(working_time));
}
