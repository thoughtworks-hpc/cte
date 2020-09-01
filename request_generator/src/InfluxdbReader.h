/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_INFLUXDBREADER_H_
#define REQUEST_GENERATOR_SRC_INFLUXDBREADER_H_
#include <string>

#include "./database_interface.h"

class InfluxdbReader : public DatabaseQueryInterface {
 public:
  std::string GetOrders(std::string ip, std::string port) override;
};

#endif  //  REQUEST_GENERATOR_SRC_INFLUXDBREADER_H_
