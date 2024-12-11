#include "store.h"
#include <limits>

bool Store::exists(const std::string &key) {
  auto it = data.find(key);
  if(it == data.end()) return false;
  if(it->second.expiry_epoch <= std::time(nullptr)) {
    del(key);
    return false;
  }
  return true;
}

void Store::set(const std::string &key, const std::string &val) {
  data[key] = { val, std::numeric_limits<std::time_t>::max() };
}

std::string Store::get(const std::string &key) {
  auto it = data.find(key);
  if(it == data.end()) return {};
  if(it->second.expiry_epoch <= std::time(nullptr)) {
    del(key);
    return {};
  }
  return data.at(key).val;
}

std::time_t Store::get_timeout(const std::string &key) {
  auto it = data.find(key);
  if(it == data.end()) return {};
  if(it->second.expiry_epoch <= std::time(nullptr)) {
    del(key);
    return {};
  }
  return data.at(key).expiry_epoch;
}

int Store::del(const std::string &key) {
  return data.erase(key);
}

int Store::set_expire(const std::string &key, const std::time_t expiry_epoch) {
  if(data.find(key) == data.end()) return 0;
  auto &val = data.at(key);
  if(expiry_epoch <= std::time(nullptr)) {
    del(key);
    return 0;
  } else {
    val.expiry_epoch = expiry_epoch;
  }
  return 1;
}
