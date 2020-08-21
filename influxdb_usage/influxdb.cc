/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "influxdb.hpp"

#include <iostream>

int main(int argc, char **argv) {
  // http write
  //创建http链接对象， 输入的参数分别是：influxDB的IP地址，端口号，DB名，用户名（可缺省）， 密码（可缺省）
  influxdb_cpp::server_info si("127.0.0.1", 8086, "simple", "", "");

  std::string resp;
  //发送写DB请求
  int ret = influxdb_cpp::builder()
                .meas("test")
                .tag("k", "v")
                .tag("x", "y")
                .field("x", 10)
                .field("y", 10.3, 2)
                .field("b", !!10)
                .timestamp(1512722735522840439)
                .post_http(si, &resp);

  //处理返回结果
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
            .timestamp(1512722735522840439)
            .send_udp("127.0.0.1", 8089);
  //udp只能判断本地发送是否成功，不能保证对端是否收到
  if (0 == ret) {
    std::cout << "send write db success" << std::endl;
  }

  // query from table
  influxdb_cpp::server_info si_new("127.0.0.1", 8086, "simple", "", "");
  ret = influxdb_cpp::query(resp, "select * from test", si);
  if (0 == ret){
    std::cout << "query db success, resp:" << resp << std::endl;
  } else {
    std::cout << "query db failed ret:" << ret << std::endl;
  }


  // create_db
  ret = influxdb_cpp::create_db(resp, "x", si_new);
  if (0 == ret){
    std::cout << "creat db success, resp:" << resp << std::endl;
  } else {
    std::cout << "creat db failed ret:" << ret << std::endl;
  }
  std::cout << resp << std::endl;
  return 0;
}