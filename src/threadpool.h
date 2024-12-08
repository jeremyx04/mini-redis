#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> jobs;
  std::mutex queue_mutex;
  std::condition_variable cv;
  bool stop;
 public:
  ThreadPool(int num_threads);
  ~ThreadPool();
  void enqueue(std::function<void()> task);
};

#endif
