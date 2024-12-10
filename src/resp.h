#ifndef RESP_H
#define RESP_H

#include <memory>
#include <string>
#include <vector>

// RESP data type 
class RType {
 public:
  virtual std::string serialize() = 0;
  virtual ~RType() = default;
  
  static std::unique_ptr<RType> deserialize(const std::string& str);
};

class SimpleString : public RType {
  std::string str;
 public:
  SimpleString(const std::string& str) : str{str} {}
  std::string serialize() override;
  const std::string& get_str() const {
    return str;
  }
};

class Error : public RType {
  std::string str; 
 public:
  Error(const std::string& str) : str{str} {}
  std::string serialize() override;
  const std::string& get_str() const {
    return str;
  }
};

class BulkString : public RType {
  std::string str;
 public:
  BulkString(const std::string& str) : str{str} {}
  std::string serialize() override;
  const std::string& get_str() const {
    return str;
  }
};

class Integer : public RType {
  int val;
 public:
  Integer(int val) : val{val} {}
  std::string serialize() override;
  const int get_val() const {
    return val;
  }
};

class Array : public RType {
  std::vector<std::unique_ptr<RType>> lst;
 public:
  Array() : lst() {}
  Array(std::vector<std::unique_ptr<RType>>& init) {
    for(auto &p : init) lst.push_back(std::move(p));
  }
  std::string serialize() override;
  const std::vector<std::unique_ptr<RType>>& get_lst() const {
    return lst;
  }
};

#endif
