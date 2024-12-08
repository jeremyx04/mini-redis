#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable cv;
  bool stop;
 public:
  ThreadPool(int num_threads);
  ~ThreadPool();
  template<typename F, typename... Args>
  auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
};

#endif

template<typename F, typename... Args>
inline auto ThreadPool::enqueue(F &&f, Args &&...args) 
  -> std::future<typename std::result_of<F(Args...)>::type>
{
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );
  auto res = task->get_future();
  
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if(stop) throw std::runtime_error("failed to enqueue on stopped ThreadPool");
    tasks.emplace([task]() { (*task)(); });
  }

  cv.notify_one();
  return res;
}
