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

std::string Array::serialize()
{
  std::string ret = "*" + std::to_string(lst.size()) + "\r\n";
  for(auto& p : lst) ret += p->serialize();
  return ret;
}
