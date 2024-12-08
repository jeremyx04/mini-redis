#include "threadpool.h"

ThreadPool::ThreadPool(int num_threads) : stop(false) {
  assert(num_threads > 0);
  for(int i = 0; i < num_threads; ++i) {
    workers.emplace_back([this] {
      while(true) {
        std::function<void()> job;
        {
          std::unique_lock<std::mutex> lock(queue_mutex);
          cv.wait(lock, [this] {
            return !jobs.empty() || stop; 
          });
          if(jobs.empty() && stop) return;
          job = std::move(jobs.front());
          jobs.pop();
        }
        job();
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

void ThreadPool::enqueue(std::function<void()> job) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    jobs.emplace(job);
  }
  cv.notify_one();
}
