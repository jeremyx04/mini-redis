#ifndef SERVER_H
#define SERVER_H

#include "redisengine.h"
#include "resp.h"
#include "store.h"
#include "threadpool.h"
#include <string>

constexpr int BUFFER_SIZE = 1024;

class Server {
  int port, server_fd;
  bool listening;
  RedisEngine redis;
  ThreadPool thread_pool;

  void handle_client(int client_fd);
 public:
  Server(int port);
  ~Server();
  void start();
  void stop();
};

#endif
