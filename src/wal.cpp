#include "wal.h"
#include <ctime>
#include <iostream>

WAL::WAL(const std::string &path) : wal_path(path) {
  wal_file.open(wal_path, std::ios::app);
  if (!wal_file.is_open()) {
    std::cerr << "Warning: Failed to open WAL file at " << wal_path << "\n";
  }
}

WAL::~WAL() {
  if (wal_file.is_open()) {
    wal_file.close();
  }
}

void WAL::log_operation(const std::string &operation) {
  std::lock_guard<std::mutex> lock(wal_mutex);
  if (!wal_file.is_open()) {
    return;  // Silently fail if file not open
  }
  wal_file << operation << "\n";
  wal_file.flush();
}

std::vector<std::string> WAL::read_log() const {
  std::vector<std::string> entries;
  std::ifstream infile(wal_path);
  if (!infile.is_open()) {
    return entries;
  }

  std::string line;
  while (std::getline(infile, line)) {
    entries.push_back(line);
  }
  infile.close();
  return entries;
}

void WAL::clear() {
  std::lock_guard<std::mutex> lock(wal_mutex);
  if (wal_file.is_open()) {
    wal_file.close();
  }
  wal_file.open(wal_path, std::ios::trunc);
  if (wal_file.is_open()) {
    wal_file.flush();
  }
}
