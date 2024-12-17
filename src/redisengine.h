#ifndef ENGINE_H
#define ENGINE_H

#include "resp.h"
#include "store.h"
#include <memory>
#include <string>

class RedisEngine {
  std::unique_ptr<Store> data_store;
  std::unique_ptr<RType> handle_command(const std::string &command, const std::vector<std::unique_ptr<RType>> &args);
 public:
  RedisEngine();
  std::unique_ptr<RType> handle_request(const std::string &req);
  bool load_data();
};

#endif

