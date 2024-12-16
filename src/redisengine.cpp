#include "redisengine.h"
#include <ctime>
#include <stdexcept>

std::unique_ptr<RType> RedisEngine::handle_request(const std::string &req) {
  std::unique_ptr<RType> resp = RType::deserialize(req);

  Array* arr = dynamic_cast<Array*>(resp.get());
  if(!arr) return std::make_unique<Error>("non-array request received");

  const std::vector<std::unique_ptr<RType>> &lst = arr->get_lst();
  if(lst.empty()) return std::make_unique<Error>("empty request received");

  BulkString* command = dynamic_cast<BulkString*>(lst[0].get());
  if(!command) return std::make_unique<Error>("non-bulk string command received");
  std::string command_string = command->get_str();
  std::transform(command_string.begin(), command_string.end(), command_string.begin(), ::toupper);
  return handle_command(command_string, lst);
}

std::unique_ptr<RType> RedisEngine::handle_command(
    const std::string &command, 
    const std::vector<std::unique_ptr<RType>> &args
) {
  size_t num_args = args.size() - 1;
  if(command == "PING") {
    if(num_args == 0) return std::make_unique<SimpleString>("PONG"); 
    if(num_args != 1) return std::make_unique<Error>("wrong number of arguments for 'ping' command");
    BulkString* message = dynamic_cast<BulkString*>(args[1].get());
    if(!message) return std::make_unique<Error>("missing message");

    return std::make_unique<BulkString>(message->get_str());
  } else if(command == "ECHO") {
    if(num_args != 1) return std::make_unique<Error>("wrong number of arguments for 'echo' command");
    BulkString* message = dynamic_cast<BulkString*>(args[1].get());
    if(!message) return std::make_unique<Error>("missing message");

    return std::make_unique<BulkString>(message->get_str());
  } else if(command == "SET") {
    if(num_args < 2) return std::make_unique<Error>("wrong number of arguments for 'set' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get()), *val = dynamic_cast<BulkString*>(args[2].get());
    if(!key) return std::make_unique<Error>("missing key");
    if(!val) return std::make_unique<Error>("missing value");

    std::string key_str = key->get_str();
    data_store->set(key_str, val->get_str());

    if(num_args > 2) {
      if(num_args == 3) return std::make_unique<Error>("wrong number of arguments for 'set' command");
      BulkString *config = dynamic_cast<BulkString*>(args[3].get()), *time = dynamic_cast<BulkString*>(args[4].get());
      if(!config) return std::make_unique<Error>("missing config");
      if(!time) return std::make_unique<Error>("missing time");
      int time_int;
      try {
        time_int = std::stoi(time->get_str());
      } catch (...) {
        throw std::runtime_error("expected an integer for time");
      }
      std::string config_str = config->get_str();
      if(config_str == "EX") {
        std::time_t expiry_epoch = std::time(nullptr) + static_cast<std::time_t>(time_int);
        data_store->set_expire(key_str, expiry_epoch);
      } else if(config_str == "PX") {
        std::time_t expiry_epoch = std::time(nullptr) + static_cast<std::time_t>(time_int / 1000);
        data_store->set_expire(key_str, expiry_epoch);
      } else if(config_str == "EXAT") {
        std::time_t expiry_epoch = static_cast<std::time_t>(time_int);
        data_store->set_expire(key_str, expiry_epoch);
      } else if(config_str == "PXAT") {
        std::time_t expiry_epoch = static_cast<std::time_t>(time_int / 1000);
        data_store->set_expire(key_str, expiry_epoch);
      } else {
        return std::make_unique<Error>("invalid config for 'set' command");
      }
    }
    return std::make_unique<BulkString>("OK");
  } else if(command == "GET") {
    if(num_args != 1) return std::make_unique<Error>("wrong number of arguments for 'get' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");
    
    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::LIST) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");

    std::string val = data_store->get(key->get_str());
    if(val.empty()) return std::make_unique<Error>("1");
    return std::make_unique<BulkString>(val);
  } else if(command == "DEL") {
    if(num_args == 0) return std::make_unique<Error>("no arguments received for 'del' command");
    int num_deleted = 0;
    for(int i = 1; i <= num_args; ++i) {
      BulkString *key = dynamic_cast<BulkString*>(args[i].get());
      if(!key) return std::make_unique<Error>("missing key");
      num_deleted += data_store->del(key->get_str());
    }
    return std::make_unique<Integer>(num_deleted);
  } else if(command == "EXISTS") {
    if(num_args == 0) return std::make_unique<Error>("no arguments received for 'exists' command");
    int num_exists = 0;
    for(int i = 1; i <= num_args; ++i) {
      BulkString *key = dynamic_cast<BulkString*>(args[i].get());
      if(!key) return std::make_unique<Error>("missing key");
      num_exists += (int) data_store->exists(key->get_str());
    }
    return std::make_unique<Integer>(num_exists);
  } else if(command == "EXPIRE") {
    if(num_args != 2) return std::make_unique<Error>("wrong number of arguments for 'expire' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    BulkString *seconds = dynamic_cast<BulkString*>(args[2].get());
    if(!key) return std::make_unique<Error>("missing key");
    if(!seconds) return std::make_unique<Error>("missing seconds");
    int seconds_int;
    try {
      seconds_int = std::stoi(seconds->get_str());
    } catch (...) {
      throw std::runtime_error("expected an integer for seconds");
    }
    std::time_t expiry_epoch = std::time(nullptr) + static_cast<std::time_t>(seconds_int);
    int res = data_store->set_expire(key->get_str(), expiry_epoch);
    return std::make_unique<Integer>(res);
  } else if(command == "TTL") {
    if(num_args != 1) return std::make_unique<Error>("wrong number of arguments for 'ttl' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");
    
    int ret;
    if(!data_store->exists(key->get_str())) {
      ret = -2;
    } else {
      std::time_t timeout = data_store->get_timeout(key->get_str());
      if(timeout == std::numeric_limits<std::time_t>::max()) {
        ret = -1;
      } else {
        // assumes that timeout > current time
        ret = (int) std::difftime(timeout, std::time(nullptr));
      }
    }
    return std::make_unique<Integer>(ret);
  } else if(command == "INCR") {
    if(num_args != 1) return std::make_unique<Error>("wrong number of arguments for 'incr' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::LIST) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");

    std::string res = data_store->incr(key->get_str(), 1);
    if(res.empty()) {
      return std::make_unique<Error>("no value at given key");
    } else if(res == "error") {
      return std::make_unique<Error>("value at given key is not an integer");
    } else {
      return std::make_unique<Integer>(std::stoi(res));
    }
  } else if(command == "DECR") {
    if(num_args != 1) return std::make_unique<Error>("wrong number of arguments for 'incr' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::LIST) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");
    
    std::string res = data_store->incr(key->get_str(), -1);
    if(res.empty()) {
      return std::make_unique<Error>("no value at given key");
    } else if(res == "error") {
      return std::make_unique<Error>("value at given key is not an integer");
    } else {
      return std::make_unique<Integer>(std::stoi(res));
    }
  } else if(command == "INCRBY") {
    if(num_args != 2) return std::make_unique<Error>("wrong number of arguments for 'incrby' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::LIST) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");
    
    BulkString *add = dynamic_cast<BulkString*>(args[2].get());
    int add_int;
    try {
      add_int = std::stoi(add->get_str());
    } catch (...) {
      return std::make_unique<Error>("increment must be an integer");
    }
    std::string res = data_store->incr(key->get_str(), add_int);
    if(res.empty()) {
      return std::make_unique<Error>("no value at given key");
    } else if(res == "error") {
      return std::make_unique<Error>("value at given key is not an integer");
    } else {
      return std::make_unique<Integer>(std::stoi(res));
    }
  } else if(command == "DECRBY") {
    if(num_args != 2) return std::make_unique<Error>("wrong number of arguments for 'decrby' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::LIST) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");
    
    BulkString *add = dynamic_cast<BulkString*>(args[2].get());
    int add_int;
    try {
      add_int = std::stoi(add->get_str());
    } catch (...) {
      return std::make_unique<Error>("increment must be an integer");
    }
    std::string res = data_store->incr(key->get_str(), -add_int);
    if(res.empty()) {
      return std::make_unique<Error>("no value at given key");
    } else if(res == "error") {
      return std::make_unique<Error>("value at given key is not an integer");
    } else {
      return std::make_unique<Integer>(std::stoi(res));
    }    
  } else if(command == "LPUSH") {
    if(num_args < 2) return std::make_unique<Error>("too few arguments for 'lpush' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");
    
    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::STRING) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");
    
    int ret = 0;
    for(int i = 2; i <= num_args; ++i) {
      BulkString *element = dynamic_cast<BulkString*>(args[i].get());
      if(!element) return std::make_unique<Error>("missing element");
      ret = data_store->lpush(key->get_str(), element->get_str());
    }
    return std::make_unique<Integer>(ret);
  } else if(command == "RPUSH") {
    if(num_args < 2) return std::make_unique<Error>("too few arguments for 'rpush' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");
    
    ValueType t = data_store->get_type(key->get_str());
    if(t == ValueType::STRING) return std::make_unique<Error>("wrong type, operation against a key holding the wrong kind of value");
    
    int ret = 0;
    for(int i = 2; i <= num_args; ++i) {
      BulkString *element = dynamic_cast<BulkString*>(args[i].get());
      if(!element) return std::make_unique<Error>("missing element");
      ret = data_store->rpush(key->get_str(), element->get_str());
    }
    return std::make_unique<Integer>(ret);
  } else if(command == "LPOP") {
    if(num_args != 1 && num_args != 2) return std::make_unique<Error>("wrong number of arguments for 'lpop' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    int count_int = 1;
    if(num_args == 2) {
      BulkString *count = dynamic_cast<BulkString*>(args[2].get());
      try {
        count_int = std::stoi(count->get_str());
      } catch (...) {
        return std::make_unique<Error>("expected integer count");
      }
    }
    std::vector<std::unique_ptr<RType>> ret;
    while(count_int-- > 0) {
      std::string res = data_store->lpop(key->get_str());
      if(res.empty()) break;
      ret.push_back(std::make_unique<BulkString>(res));
    }
    return ret.size() == 1 ? std::move(ret[0]) : std::make_unique<Array>(ret);
  } else if(command == "RPOP") {
    if(num_args != 1 && num_args != 2) return std::make_unique<Error>("wrong number of arguments for 'rpop' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    int count_int = 1;
    if(num_args == 2) {
      BulkString *count = dynamic_cast<BulkString*>(args[2].get());
      try {
        count_int = std::stoi(count->get_str());
      } catch (...) {
        return std::make_unique<Error>("expected integer count");
      }
    }
    std::vector<std::unique_ptr<RType>> ret;
    while(count_int-- > 0) {
      std::string res = data_store->rpop(key->get_str());
      if(res.empty()) break;
      ret.push_back(std::make_unique<BulkString>(res));
    }
    return ret.size() == 1 ? std::move(ret[0]) : std::make_unique<Array>(ret);
  } else if(command == "LINDEX") {
    if(num_args != 2) return std::make_unique<Error>("wrong number of arguments for 'lindex' command");
    BulkString *key = dynamic_cast<BulkString*>(args[1].get());
    if(!key) return std::make_unique<Error>("missing key");

    int index_int;
    BulkString *index = dynamic_cast<BulkString*>(args[2].get());
    try {
      index_int = std::stoi(index->get_str());
    } catch (...) {
      return std::make_unique<Error>("expected integer index");
    }
    std::string res = data_store->lindex(key->get_str(),index_int);
    if(res.empty()) return std::make_unique<Error>("invalid index");
    return std::make_unique<BulkString>(res);
  } else {
    return std::make_unique<Error>(std::string("unrecognized command " + command));
  }
}

RedisEngine::RedisEngine() {
  data_store = std::make_unique<Store>();
}
