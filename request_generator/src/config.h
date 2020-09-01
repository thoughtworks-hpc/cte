//
// Created by Yuecheng Pei on 2020/9/1.
//

#ifndef REQUEST_GENERATOR_SRC_CONFIG_H_
#define REQUEST_GENERATOR_SRC_CONFIG_H_

/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "../../common/include/json.hpp"
#include "src/generator.h"

namespace request_generator {
class Config {
 public:
  int default_num_of_threads_;
  int default_num_of_requests_;
  std::string default_db_host_address_;
  std::string default_db_port_;
  std::vector<ip_address> grcp_ip_address_;
  std::string config_file_path_;

  explicit Config(std::string config_file_path)
      : config_file_path_(std::move(config_file_path)) {
    nlohmann::json config;
    std::ifstream input(config_file_path_);
    input >> config;

    default_num_of_threads_ = config["default_num_of_threads"];
    default_num_of_requests_ = config["default_num_of_requests"];
    default_db_host_address_ = config["default_db_host_address"];
    default_db_port_ = config["default_db_port"];

    for (int i = 0; i < config["grpc_server"].size(); i++) {
      grcp_ip_address_.emplace_back(
          static_cast<std::string>(config["grpc_server"][i]["ip"]),
          static_cast<std::string>(config["grpc_server"][i]["port"]));
    }
  }
};
}  // namespace request_generator

#endif  // REQUEST_GENERATOR_SRC_CONFIG_H_
