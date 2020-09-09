/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "include/Influxdb_reader.h"

#include <iostream>
#include <string>
#include <utility>

std::string InfluxdbReader::GetOrders() {
  std::string resp;

  int ret =
      influxdb_cpp::query(resp, "select  * from orders order by time", si_);
  if (0 == ret) {
    std::cout << "[INFO] query db success" << std::endl;
  } else {
    std::cout << "[ERROR] query db failed ret:" << ret << std::endl;
    return "";
  }
  // std::cout << resp << std::endl;
  return resp;
}
