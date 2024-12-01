#include "store.h"

std::unordered_map<std::string, std::string> Store::data;

void Store::set(const std::string &key, const std::string &val) {
  data[key] = val;
}

void Store::get(const std::string &key, std::string &val) const {
  if(data.find(key) == data.end()) return;
  val = data[key];
}
