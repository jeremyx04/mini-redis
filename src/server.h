#ifndef SERVER_H
#define SERVER_H

constexpr int BUFFER_SIZE = 1024;

class Server {
  int port, server_fd;
  bool listening;
 public:
  Server(int port);
  ~Server();

  void start();
  void stop();
};

#endif
