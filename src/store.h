#ifndef STORE_H
#define STORE_H

#include <string>
#include <unordered_map>

class Store {
  static std::unordered_map<std::string, std::string> data;
 public:
  void set(const std::string &key, const std::string &val);
  void get(const std::string &key, std::string &val) const;
};

#endif
