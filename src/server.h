#ifndef SERVER_H
#define SERVER_H

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
