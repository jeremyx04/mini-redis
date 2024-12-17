#ifndef STORE_H
#define STORE_H

#include "linkedlist.h"
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>

enum class ValueType {
  STRING,
  LIST,
  EMPTY,
};

struct Value {
  std::variant<std::string, LinkedList> val;
  std::time_t expiry_epoch;
};

class Store {
  std::unordered_map<std::string, Value> data;
  std::shared_mutex data_mutex;
 public:
  std::unordered_map<std::string, Value> get_data() const { return data; }
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
  // Increment the value at key by add, returns the result as a base 10 integer
  // or "error" if the value is not an integer
  std::string incr(const std::string &key, int add);
  // Returns the type of the value at key
  ValueType get_type(const std::string &key);

  // *List functions assume that value stored at key is not a string*

  // Push element at head of list stored at key and
  // returns length of the list after insertion
  size_t lpush(const std::string &key, const std::string &element);
  // Push element at tail of list stored at key and
  // returns length of the list after insertion
  size_t rpush(const std::string &key, const std::string &element);
  // Removes and returns the first element of the list stored at key
  std::string lpop(const std::string &key);
  // Removes and returns the last element of the list stored at key
  std::string rpop(const std::string &key);
  // Returns value at given index of the list stored at key
  std::string lindex(const std::string &key, int idx);
  // Returns length of list stored at key
  size_t llen(const std::string &key);
};

#endif
