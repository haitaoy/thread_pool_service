#include <iostream>
#include <thread>
#include <functional>
#include "../blocking_queue.h"

const int size = 100;

struct Producer {
  void operator()(BlockingQueue<int> &queue) {
    for (int i = 0; i < size; ++i) {
      queue.Push(i + 1);
    }
  }
};

struct Consumer {
  void operator()(BlockingQueue<int> &queue) {
    while (true) {
      int item = *(queue.WaitAndPop());
      std::cout << "get item " << item << " from queue\n";
    }
  }
};

int main() {

  std::shared_ptr<BlockingQueue<int> > int_queue(new BlockingQueue<int>(10));

  std::thread producer1(Producer(), std::ref(*int_queue));
  std::thread consumer1(Consumer(), std::ref(*int_queue));
  std::thread consumer2(Consumer(), std::ref(*int_queue));

  producer1.join();

  /** consumers are blocked, useless */
  consumer1.join();
  consumer2.join();

  return 0;
}

