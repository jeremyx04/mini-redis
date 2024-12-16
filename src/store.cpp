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
      ret = std::get<std::string>(it->second.val);
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
    val_int = std::stoi(std::get<std::string>(val.val));
  } catch (...) {
    return "error";
  }
  val.val = std::to_string(val_int + add);
  return std::get<std::string>(val.val);
}

ValueType Store::get_type(const std::string &key) {
  std::shared_lock<std::shared_mutex> lock(data_mutex);
  auto it = data.find(key);
  if(it == data.end()) {
    return ValueType::EMPTY;
  }
  auto &val = it->second.val;
  return val.index() == 0 ? ValueType::STRING : ValueType::LIST;
}

size_t Store::lpush(const std::string &key, const std::string &element) {
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  auto it = data.find(key);
  Value &val = (it == data.end()) 
                  ? data[key] = {LinkedList(), std::numeric_limits<std::time_t>::max()}
                  : it->second;
  if(val.expiry_epoch <= std::time(nullptr)) {
    data.erase(key);
    return 0;
  } 
  LinkedList &lst = std::get<LinkedList>(val.val);
  lst.push_front(element);
  return lst.get_size();
}

size_t Store::rpush(const std::string &key, const std::string &element) {
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  auto it = data.find(key);
  Value &val = (it == data.end()) 
                  ? data[key] = {LinkedList(), std::numeric_limits<std::time_t>::max()}
                  : it->second;
  if(val.expiry_epoch <= std::time(nullptr)) {
    data.erase(key);
    return 0;
  } 
  LinkedList &lst = std::get<LinkedList>(val.val);
  lst.push_back(element);
  return lst.get_size();
}

std::string Store::lpop(const std::string &key) {
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  auto it = data.find(key);
  if(it == data.end()) return {};
  Value &val = it->second;
  if(val.expiry_epoch <= std::time(nullptr)) {
    data.erase(key);
    return 0;
  } 
  LinkedList &lst = std::get<LinkedList>(val.val);
  return lst.pop_front();
}

std::string Store::rpop(const std::string &key) {
  std::unique_lock<std::shared_mutex> lock(data_mutex);
  auto it = data.find(key);
  if(it == data.end()) return {};
  Value &val = it->second;
  if(val.expiry_epoch <= std::time(nullptr)) {
    data.erase(key);
    return 0;
  } 
  LinkedList &lst = std::get<LinkedList>(val.val);
  return lst.pop_back();
}

std::string Store::lindex(const std::string &key, int idx) {
  bool remove_flag = false;
  std::string ret;
  {
    std::shared_lock<std::shared_mutex> lock(data_mutex);
    auto it = data.find(key);
    if(it == data.end()) return ret;
    Value &val = it->second;
    if(val.expiry_epoch <= std::time(nullptr)) {
      remove_flag = true;
    }
    LinkedList &lst = std::get<LinkedList>(val.val);
    ret = lst.at(idx);
  }
  if(remove_flag) {
    del(key);
  }
  return ret;
}
