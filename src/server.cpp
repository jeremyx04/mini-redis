#include "server.h"

#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

void Server::handle_client(int client_fd) {
  std::cout << "client connected\n";
  while(true) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if(bytes_received < 0) {
      std::cerr << "failed to receive bytes from client\n";
      break;
    } else if(bytes_received == 0) {
      break;
    } else {
      buffer[BUFFER_SIZE-1] = '\0';
      std::string received(buffer);
      std::unique_ptr<RType> res = redis.handle_request(received);
      std::string response_string = res->serialize();
      send(client_fd, response_string.c_str(), response_string.size(), 0);
    }
  }
}

Server::Server(int port) : port(port), listening(false), thread_pool(50) {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0) {
    throw std::runtime_error("socket creation failed");
  }

  const int enable = 1;
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    throw std::runtime_error("set reuse address option failed");
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    throw std::runtime_error("set reuse port option failed");

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    std::string error_msg = "failed to bind to port " + std::to_string(port) + ": " + strerror(errno);
    throw std::runtime_error(error_msg);
  }
}

Server::~Server() {
  stop();
}

void Server::start() {
  if(listen(server_fd, SOMAXCONN) < 0) {
    throw std::runtime_error("server listen failed");
  }
  redis = RedisEngine();
  listening = true;

  while(listening) {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if(client_fd < 0) {
      std::cerr << "failed to accept client connection\n";
      continue;
    }
    thread_pool.enqueue([this, client_fd] {
        handle_client(client_fd);
        close(client_fd);
        std::cout << "client disconnected\n";
    });
  }
}

void Server::stop() {
  listening = false;
  close(server_fd);
}
