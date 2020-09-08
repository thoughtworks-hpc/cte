/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/config.h"

#include <unistd.h>

std::vector<std::string> trade_persistence_tool::ParseIpAddress(
    std::string address) {
  std::vector<std::string> output;
  std::string ip;
  std::string port;
  for (int i = 0; i < address.size(); i++) {
    if (address[i] == ':') {
      port += address[i + 1];
      i = i + 2;
    }
    if (port.empty()) {
      ip += address[i];
    } else {
      port += address[i];
    }
  }
  output.push_back(ip);
  output.push_back(port);
  return output;
}

trade_persistence_tool::Config::Config(int argc, char* argv[]) {
  database_name = "trade_manager";
  database_table_name = "trades";
  cdcf_config.log_file_ = "/tmp/trade_persistence_tool.log";
  int option;
  while ((option = getopt(argc, argv, "m:d:n:l:u:p:f:t:")) != -1) {
    switch (option) {
      case 'm':
        order_manager_address = optarg;
        break;
      case 'd':
        db_ip = ParseIpAddress(optarg)[0];
        db_port = ParseIpAddress(optarg)[1];
        break;
      case 'n':
        database_name = optarg;
        break;
      case 'p':
        db_password = optarg;
        break;
      case 'u':
        db_username = optarg;
        break;
      case 'f':
        cdcf_config.log_file_ = optarg;
        break;
      case 'l':
        cdcf_config.log_level_ = optarg;
        break;
      case 't':
        database_table_name = optarg;
        break;
      case '?':
        printf("Unknown option:  %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }
  cdcf::Logger::Init(cdcf_config);
}
