/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "Influxdb_reader.h"

#include <iostream>
#include <string>

#include "../../common/include/influxdb.hpp"

std::string Influxdb_reader::GetOrders(std::string ip, std::string port) {
  std::string resp;
  influxdb_cpp::server_info si(ip, std::stoi(port), "orders", "", "");
  int ret = influxdb_cpp::query(resp, "select  * from orders", si);
  if (0 == ret) {
    std::cout << "[INFO] query db success" << std::endl;
  } else {
    std::cout << "[ERROR] query db failed ret:" << ret << std::endl;
    return "";
  }
  // std::cout << resp << std::endl;
  return resp;
}