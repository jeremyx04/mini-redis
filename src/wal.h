#ifndef WAL_H
#define WAL_H

#include <string>
#include <vector>
#include <mutex>
#include <fstream>

class WAL {
  std::ofstream wal_file;
  std::mutex wal_mutex;
  std::string wal_path;

 public:
  WAL(const std::string &path = "redis.wal");
  ~WAL();

  // Log an operation atomically
  void log_operation(const std::string &operation);

  // Read all log entries
  std::vector<std::string> read_log() const;

  // Clear the WAL file
  void clear();
};

#endif
