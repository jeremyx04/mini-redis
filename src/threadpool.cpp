#include "threadpool.h"

ThreadPool::ThreadPool(int num_threads) : stop(false) {
  assert(num_threads > 0);
  for(int i = 0; i < num_threads; ++i) {
    workers.emplace_back([this] {
      while(true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex);
          cv.wait(lock, [this] {
            return !tasks.empty() || stop; 
          });
          if(tasks.empty() && stop) return;
          task = std::move(tasks.front());
          tasks.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  cv.notify_all();
  for(auto& w : workers) {
    if(w.joinable()) w.join();
  }
}
