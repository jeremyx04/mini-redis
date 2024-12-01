#include "resp.h"

std::string SimpleString::serialize() {
  return "+" + str + "\r\n";
}

std::string Error::serialize() {
  return "-" + str + "\r\n";
}

std::string BulkString::serialize() {
  return "$" + std::to_string(str.size()) + "\r\n" + str + "\r\n";
}

std::string Integer::serialize() {
  return ":" + std::to_string(val) + "\r\n";
}

std::string Array::serialize() {
  std::string ret = "*" + std::to_string(lst.size()) + "\r\n";
  for(auto& p : lst) ret += p->serialize();
  return ret;
}

// remove trailing \r\n characters
std::string trim(const std::string &str) {
    std::string res = str;
    while (res.size() >= 2 && res.substr(res.size() - 2) == "\r\n") {
        res.erase(res.size() - 2); 
    }
    return res;
}

std::unique_ptr<RType> RType::deserialize(const std::string &str) {
  if (str.size() == 0) return std::unique_ptr<RType>(nullptr);
  char type = str[0];
  switch (type) {
    case '+': { 
      return std::make_unique<SimpleString>(trim(str.substr(1)));
    }
    case '-': {
      return std::make_unique<Error>(trim(str.substr(1)));
    }
    case '$': {
      size_t pos = str.find("\r\n", 1);
      if(pos == std::string::npos) {
        throw std::invalid_argument("invalid bulk string");
      }
      int len = std::stoi(str.substr(1, pos-1));
      std::string bulk_str = str.substr(pos+2, len);
      return std::make_unique<BulkString>(trim(bulk_str));
    }
    case ':': {
      int val = std::stoi(str.substr(1));
      return std::make_unique<Integer>(val);
    }
    case '*': {
      size_t pos = str.find("\r\n", 1);
      if(pos == std::string::npos) {
          throw std::invalid_argument("invalid array");
      }
      int n = std::stoi(str.substr(1, pos-1));
      std::vector<std::unique_ptr<RType>> lst;
      size_t start = pos + 2;  
      for (int i = 0; i < n; i++) {
          // bulk strings have 2 \r\n instances...
          size_t end = str.find("\r\n", start);
          if(end == std::string::npos) {
              throw std::invalid_argument("invalid array element");
          }
          bool is_bulk_string = str[start] == '$';
          if(is_bulk_string) {
            end = str.find("\r\n", end + 2);
            if(end == std::string::npos) {
              throw std::invalid_argument("invalid bulk string element in array");
            }
          }
          std::string element = str.substr(start, end - start);
          lst.push_back(RType::deserialize(element));
          start = end + 2; 
      }
      return std::make_unique<Array>(lst);
    }
    default:
      throw std::invalid_argument("unrecognized RESP format");
  }
}
