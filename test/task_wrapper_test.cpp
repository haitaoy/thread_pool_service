#include <iostream>
#include <thread>
#include "../task_wrapper.h"

struct Printer {
  void operator()() {
    std::cout << "print from struct\n";
  }
};

void print() {
  std::cout << "print from function\n";
}

int main() {
  TaskWrapper *task1 = new TaskWrapper(Printer());
  TaskWrapper task2(print);

  task1->operator()();
  task2.operator()();

//  std::thread t1(task2);
//  std::thread t2(task2);
//
//  t1.join();
//  t2.join();

  return 0;
}