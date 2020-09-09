/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_INFLUXDB_READER_H_
#define REQUEST_GENERATOR_SRC_INFLUXDB_READER_H_
#include <iostream>
#include <string>

#include "../../common/include/influxdb.hpp"
#include "./database_interface.h"

class InfluxdbReader : public DatabaseQueryInterface {
 public:
  influxdb_cpp::server_info& si_;

  explicit InfluxdbReader(influxdb_cpp::server_info& si) : si_(si) {}

  std::string GetOrders() override;
};

#endif  //  REQUEST_GENERATOR_SRC_INFLUXDB_READER_H_
