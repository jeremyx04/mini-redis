#include "server.h"
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

std::unique_ptr<RType> Server::handle_request(const std::string &req) {
  std::unique_ptr<RType> resp = RType::deserialize(req);

  Array* arr = dynamic_cast<Array*>(resp.get());
  if(!arr) return std::make_unique<Error>("non-array request received");

  const std::vector<std::unique_ptr<RType>>& lst = arr->get_lst();
  if(lst.empty()) return std::make_unique<Error>("empty request received");

  BulkString* command = dynamic_cast<BulkString*>(lst[0].get());
  if(!command) return std::make_unique<Error>("non-bulk string command received");
  std::string command_string = command->get_str();
  std::transform(command_string.begin(), command_string.end(), command_string.begin(), ::toupper);
  return handle_command(command_string, lst);
}

std::unique_ptr<RType> Server::handle_command(
    const std::string &command, 
    const std::vector<std::unique_ptr<RType>> &args
) {
  size_t num_args = args.size() - 1;
  if(command == "PING") {
    if(num_args == 0) {
      return std::make_unique<SimpleString>("PONG");
    } else {
      if(num_args > 1) return std::make_unique<Error>("wrong number of arguments for 'ping' command");
      BulkString* message = dynamic_cast<BulkString*>(args[1].get());
      if(!message) return std::make_unique<Error>("missing message");
      return std::make_unique<SimpleString>(message->get_str());
    }
  } else if(command == "ECHO") {
    if(num_args > 1) return std::make_unique<Error>("wrong number of arguments for 'echo' command");
    BulkString* message = dynamic_cast<BulkString*>(args[1].get());
    if(!message) return std::make_unique<Error>("missing message");
    return std::make_unique<SimpleString>(message->get_str());
  } else if(command == "SET") {
    if(num_args > 2) return std::make_unique<Error>("wrong number of arguments for 'set' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get()), *val = dynamic_cast<BulkString*>(args[2].get());
    if(!key) return std::make_unique<Error>("missing key");
    if(!val) {
      return std::make_unique<Error>("missing value");
    }
    data_store->set(key->get_str(), val->get_str());
    return std::make_unique<SimpleString>("OK");
  } else if(command == "GET") {
    if(num_args > 1) return std::make_unique<Error>("wrong number of arguments for 'get' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");
    std::string val;
    data_store->get(key->get_str(), val);
    if(val.empty()) return std::make_unique<Error>("1");
    return std::make_unique<SimpleString>(val);
  } else {
    return std::make_unique<Error>(std::string("unrecognized command " + command));
  }
}

void Server::handle_client(int client_fd) {
  std::cout << "client connected\n";
  while(true) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if(bytes_received < 0) {
      std::cerr << "failed to receive bytes from client\n";
      break;
    } else if(bytes_received == 0) {
      std::cout << "client disconnected\n";
      break;
    } else {
      buffer[BUFFER_SIZE-1] = '\0';
      std::string received(buffer);
      std::unique_ptr<RType> res = handle_request(received);
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

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    throw std::runtime_error("failed to bind to port " + std::to_string(port));
  }
}

Server::~Server() {
  stop();
}

void Server::start() {
  int backlog = 5;
  if(listen(server_fd, backlog) < 0) {
    throw std::runtime_error("server listen failed");
  }
  data_store = new Store();
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
    });
  }
}

void Server::stop() {
  listening = false;
  close(server_fd);
}
