#include <iostream>
#include <thread>
#include "../../task_wrapper.h"

struct Printer {
  void operator()() {
    std::cout << "print from struct\n";
  }
};

void print() {
  std::cout << "print from function\n";
}

int main() {
  std::shared_ptr<TaskWrapper> task1(new TaskWrapper(Printer()));
  TaskWrapper task2(print);

  task1->operator()();
  task2.operator()();

  return 0;
}