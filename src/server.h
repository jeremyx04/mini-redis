#ifndef SERVER_H
#define SERVER_H

#include "resp.h"
#include <string>

constexpr int BUFFER_SIZE = 1024;

class Server {
  int port, server_fd;
  bool listening;
  void handle_client(int client_fd);
  std::unique_ptr<RType> handle_request(const std::string &req);
 public:
  Server(int port);
  ~Server();

  void start();
  void stop();
};

#endif
