#include <iostream>
#include <thread>
#include <atomic>
#include "../../blocking_queue.h"

const int size = 100;
std::atomic<int> product_item(0);

struct Producer {
  void operator()(BlockingQueue<int> &queue) {
    for (int i = 0; i < size; ++i) {
      int value = product_item.fetch_add(1);
      if (!queue.Push(value)) {
        std::cout << "failed to push item " << value << " into queue\n";
      }
    }
  }
};

struct ProducerWaiter {
  void operator()(BlockingQueue<int> &queue) {
    for (int i = 0; i < size; ++i) {
      queue.WaitAndPush(product_item.fetch_add(1));
    }
  }
};

struct ConsumerWaiter {
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
  std::thread producer2(ProducerWaiter(), std::ref(*int_queue));
  std::thread consumer1(ConsumerWaiter(), std::ref(*int_queue));
  std::thread consumer2(ConsumerWaiter(), std::ref(*int_queue));

  /** producers and consumers are blocked, useless */
  producer1.join();
  producer2.join();

  consumer1.join();
  consumer2.join();

  return 0;
}

