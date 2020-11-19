/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../common/include/influxdb.hpp"

#include <iostream>

int main(int argc, char **argv) {
  // http write
  influxdb_cpp::server_info si("127.0.0.1", 8086, "simple", "", "");

  std::string resp;
  int ret = influxdb_cpp::builder()
                .meas("test")
                .tag("k", "v")
                .tag("x", "y")
                .field("x", 10)
                .field("y", 10.3, 2)
                .field("b", !!10)
                // .timestamp(1512722735522840439)
                .post_http(si, &resp);

  if (0 == ret && "" == resp) {
    std::cout << "write db success" << std::endl;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
  }

  // udp write
  ret = influxdb_cpp::builder()
            .meas("test")
            .tag("k", "v")
            .tag("x", "y")
            .field("x", 10)
            .field("y", 3.14e18, 3)
            .field("b", !!10)
            // .timestamp(1512722735522840439)
            .send_udp("127.0.0.1", 8089);

  if (0 == ret) {
    std::cout << "send write db success" << std::endl;
  }

  // query from table
  influxdb_cpp::server_info si_new("127.0.0.1", 8086, "simple", "", "");
  ret = influxdb_cpp::query(resp, "select * from test", si);
  if (0 == ret) {
    std::cout << "query db success, resp:" << resp << std::endl;
  } else {
    std::cout << "query db failed ret:" << ret << std::endl;
  }

  // create_db
  ret = influxdb_cpp::create_db(resp, "x", si_new);
  if (0 == ret) {
    std::cout << "creat db success, resp:" << resp << std::endl;
  } else {
    std::cout << "creat db failed ret:" << ret << std::endl;
  }
  // std::cout << resp << std::endl;

  // insert chunk data into single table
  influxdb_cpp::detail::field_caller containt;
  containt.meas("cpu_load_short")
      .field("host", 1)
      .field("region", 1)
      .field("value", 1)
      .timestamp(76543211)
      .meas("cpu_load_short")
      .field("host", 2)
      .field("region", 2)
      .field("value", 2)
      .timestamp(12341567);

  ret = containt.meas("cpu_load_short")
            .field("host", 3)
            .field("region", 3)
            .field("value", 3)
            .timestamp(88188)
            .post_http(si, &resp);

  if (0 == ret && "" == resp) {
    std::cout << "write db success" << std::endl;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
  }

  return 0;
}
