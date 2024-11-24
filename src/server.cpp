#include "server.h"
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Server::Server(int port) : port(port), listening(false) {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0) {
    throw std::runtime_error("Socket creation failed");
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    throw std::runtime_error("Failed to bind to port " + std::to_string(port));
  }

  int backlog = 5;
  if(listen(server_fd, backlog) < 0) {
    throw std::runtime_error("Server listen failed");
  }
}

Server::~Server() {
  stop();
}

void Server::start() {
  listening = true;
  while(listening) {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if(client_fd < 0) {
      if(listening) {
        std::cerr << "Failed to accept client connection\n";
      }
      continue;
    }
    std::cout << "Client connected\n";
  }
}

void Server::stop() {
  listening = false;
  close(server_fd);
}
