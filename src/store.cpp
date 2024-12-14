#include "store.h"
#include <limits>

bool Store::exists(const std::string &key) {
  bool remove_flag = false; 
  bool ret = false;
  {
    std::unique_lock<std::shared_mutex> lock(data_mutex); 
    auto it = data.find(key);
    if(it == data.end()) return false;
    if(it->second.expiry_epoch <= std::time(nullptr)) {
      remove_flag = true;
    } else {
      ret = true;
    }
  }
  if(remove_flag) {
    del(key);
  }
  return ret;
}

void Store::set(const std::string &key, const std::string &val) {
  std::unique_lock<std::shared_mutex> lock(data_mutex); 
  data[key] = { val, std::numeric_limits<std::time_t>::max() };
}

std::string Store::get(const std::string &key) {
  bool remove_flag = false;
  std::string ret;
  {
    std::shared_lock<std::shared_mutex> lock(data_mutex);
    auto it = data.find(key);
    if(it == data.end()) return {};
    if(it->second.expiry_epoch <= std::time(nullptr)) {
      remove_flag = true;
    } else {
      ret = it->second.val;
    }
  }
  if(remove_flag) {
    del(key);
  }
  return ret;
}


std::time_t Store::get_timeout(const std::string &key) {
  bool remove_flag = false;
  std::time_t ret;
  {
  std::shared_lock<std::shared_mutex> lock(data_mutex);
  auto it = data.find(key);
  if(it == data.end()) return {};
    if(it->second.expiry_epoch <= std::time(nullptr)) {
      remove_flag = true;
    } else {
      ret = it->second.expiry_epoch;
    }
  }
  if(remove_flag) {
    del(key);
  }
  return ret;
}

int Store::del(const std::string &key) {
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  return data.erase(key);
}

int Store::set_expire(const std::string &key, const std::time_t expiry_epoch) {
  if(!exists(key)) return 0;
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  auto &val = data.at(key);
  if(expiry_epoch <= std::time(nullptr)) {
    data.erase(key);
    return 0;
  } 
  val.expiry_epoch = expiry_epoch;
  return 1;
}

std::string Store::incr(const std::string &key, const int add) {
  if(!exists(key)) {
    std::string val = std::to_string(add);
    set(key, val);
    return val;
  }
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  auto &val = data.at(key);
  if(val.expiry_epoch <= std::time(nullptr)) {
    std::string val = std::to_string(add);
    data[key].val = val;
    return val;
  }
  int val_int;
  try {
    val_int = std::stoi(val.val);
  } catch (...) {
    return "error";
  }
  val.val = std::to_string(val_int + add);
  return val.val;
}
