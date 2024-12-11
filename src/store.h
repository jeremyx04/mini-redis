#ifndef STORE_H
#define STORE_H

#include <string>
#include <unordered_map>

struct Value {
  std::string val;
  std::time_t expiry_epoch;
};

class Store {
  std::unordered_map<std::string, Value> data;
 public:
  // Returns true if specified key exists, false otherwise
  bool exists(const std::string &key);
  // Set key to value
  void set(const std::string &key, const std::string &val);
  // Get value at the specified key and returns it if it exists
  std::string get(const std::string &key);
  // Get timeout at the specified key and returns it if it exists
  std::time_t get_timeout(const std::string &key);
  // Delete the specified key and return number of keys deleted (0 or 1)
  int del(const std::string &key);
  // Set the specified key to timeout
  int set_expire(const std::string &key, const std::time_t expiry_epoch);
};

#endif
