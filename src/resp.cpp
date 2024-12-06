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

// helper fn to deserialize array types
// pass current index in str to iteratively scan through nested arrays
std::unique_ptr<RType> deserialize_array(const std::string &str, int &index) {
  assert(str[index] == '*');
  ++index; // skip the first * character

  size_t pos = str.find("\r\n", index);
  if(pos == std::string::npos) throw std::invalid_argument("invalid array");
  int num_elements;
  try {
    num_elements = std::stoi(str.substr(index, pos-1));
  } catch (...) {
    throw std::invalid_argument("invalid array length");
  }
  int cur_elements = 0;
  std::vector<std::unique_ptr<RType>> lst;
  // fill in the array with the correct number of elements
  // index should not go over size but... sanity check
  while(cur_elements < num_elements && index < str.size()) {
    if(str[index] == '+' || str[index] == '-' || str[index] == ':') {
      size_t end = str.find("\r\n", index+1);
      if(end == std::string::npos) throw std::invalid_argument("invalid array element");
      std::string element = str.substr(index, end-index);
      lst.push_back(RType::deserialize(element));
      ++cur_elements;
    } else if(str[index] == '$') {
      size_t end = str.find("\r\n", index+1);
      end = str.find("\r\n", end+1);
      if(end == std::string::npos) throw std::invalid_argument("invalid array element");
      std::string element = str.substr(index, end-index);
      lst.push_back(RType::deserialize(element));
      ++cur_elements;
      index = end;
    } else if(str[index] == '*') {
      lst.push_back(deserialize_array(str, index));
      ++cur_elements;
    }
    ++index;
  }
  return std::make_unique<Array>(lst);
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
      if(pos == std::string::npos) throw std::invalid_argument("invalid bulk string");
      int len;
      try {
        len = std::stoi(str.substr(1, pos-1));
      } catch (...) {
        throw std::invalid_argument("invalid bulk string length");
      }
      std::string bulk_str = str.substr(pos+2, len);
      return std::make_unique<BulkString>(trim(bulk_str));
    }
    case ':': {
      try {
        int val = std::stoi(str.substr(1));
        return std::make_unique<Integer>(val);
      } catch (...) {
        throw std::invalid_argument("invalid integer");
      }
    }
    case '*': {
      int index = 0;
      return deserialize_array(str, index);
    }
    default:
      throw std::invalid_argument("unrecognized RESP format");
  }
}
