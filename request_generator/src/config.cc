/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "include/config.h"

#include <unistd.h>

#include <queue>
#include <string>
#include <utility>

#include "../../common/include/json.hpp"

request_generator::Config::Config(int argc, char **argv) {
  int cli_input_num_of_threads_ = -1;
  int cli_input_num_of_requests_ = -1;
  config_file_path_ = "request_generator_config.json";

  int option;
  std::queue<order_manager_proto::Order> orders;
  while ((option = getopt(argc, argv, "t:n:f:")) != -1) {
    switch (option) {
      case 't':
        cli_input_num_of_threads_ = std::stoi(optarg);
        break;
      case 'n':
        cli_input_num_of_requests_ = std::stoi(optarg);
        break;
      case 'f':
        config_file_path_ = optarg;
        break;
      case '?':
        printf("Unknown option:  %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }

  nlohmann::json config_json;
  std::ifstream input(config_file_path_);
  input >> config_json;

  if (cli_input_num_of_threads_ == -1) {
    num_of_threads_ = config_json["default_num_of_threads"];
  } else {
    num_of_threads_ = cli_input_num_of_threads_;
  }

  if (cli_input_num_of_requests_ == -1) {
    num_of_requests_ = config_json["default_num_of_requests"];
  } else {
    num_of_requests_ = cli_input_num_of_requests_;
  }

  if (strcmp(config_json["database_user_name"].type_name(), "null") != 0) {
    database_user_name_ = config_json["database_user_name"];
  } else {
    database_user_name_ = "";
  }

  if (strcmp(config_json["database_password"].type_name(), "null") != 0) {
    database_password_ = config_json["database_password"];
  } else {
    database_password_ = "";
  }

  database_name_ = config_json["default_db_name"];
  database_ip_ = config_json["default_db_ip_address"];
  database_port_ = config_json["default_db_port"];

  for (int i = 0; i < config_json["grpc_server"].size(); i++) {
    grcp_ip_address_.emplace_back(
        static_cast<std::string>(config_json["grpc_server"][i]["ip"]),
        static_cast<std::string>(config_json["grpc_server"][i]["port"]));
  }
  std::cout << "[INFO] Build CONFIG successfully" << std::endl;
}
