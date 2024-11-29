#include <iostream>
#include "server.h"

int main(int argc, char **argv) {
  try {
    Server server(6379);
    server.start();
  } catch(const std::exception& e) {
    std::cerr << "An error occurred: " << e.what() << '\n';
  }
}
