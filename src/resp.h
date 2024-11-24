#ifndef RESP_H
#define RESP_H

#include <memory>
#include <string>
#include <vector>

class RType {
 public:
  virtual std::string serialize() = 0;
  virtual ~RType() = default;
};

class SimpleString : public RType {
  std::string str;
 public:
  SimpleString(const std::string& str) : str{str} {}
  std::string serialize() override;
};

class Error : public RType {
  std::string str; 
 public:
  Error(const std::string& str) : str{str} {}
  std::string serialize() override;
};

class BulkString : public RType {
  std::string str; 
 public:
  BulkString(const std::string& str) : str{str} {}
  std::string serialize() override;
};

class Integer : public RType {
  int val;
 public:
  Integer(int val) : val{val} {}
  std::string serialize() override;
};

class Array : public RType {
  std::vector<std::unique_ptr<RType>> lst;
 public:
  Array() : lst() {}
  Array(std::vector<std::unique_ptr<RType>>& init) {
    for(auto &p : init) lst.push_back(std::move(p));
  }
  std::string serialize() override;
};

#endif
