/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_DATABASE_INTERFACE_HPP
#define CTE_DATABASE_INTERFACE_HPP

#include <thread>
#include <vector>

#include "./influxdb.hpp"

namespace database_interface {

struct data_pair {
  std::string Key;
  std::string Value;
};

struct entity {
  std::string measurement;
  std::vector<data_pair> tag;
  std::vector<data_pair> field;
  int64_t timestamp;
};

class Database {
 public:
  virtual bool write(entity& data_entity) = 0;
  virtual ~Database(){};
};

class InfluxDB : public Database {
 public:
  InfluxDB(influxdb_cpp::server_info& si, const int default_size = 20000);

  ~InfluxDB();

  bool write(entity& data_entity);

 private:
  void thread_main();
  bool thread_write_db();
  influxdb_cpp::server_info si;
  std::vector<entity> buffer;
  std::mutex buffer_mutex;
  std::thread th;
  bool thread_end_flag;
  int count;
};

}  // namespace database_interface

#endif  // CTE_DATABASE_INTERFACE_HPP
