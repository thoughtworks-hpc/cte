/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_CONFIG_H_
#define REQUEST_GENERATOR_SRC_CONFIG_H_

/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include <fstream>
#include <string>
#include <vector>

#include "generator.h"

namespace request_generator {
class Config {
 public:
  int num_of_threads_;
  int num_of_requests_;
  std::string database_ip_;
  std::string database_port_;
  std::string database_user_name_;
  std::string database_password_;
  std::vector<ip_address> grcp_ip_address_;
  std::string config_file_path_;
  std::string database_name_;

  explicit Config(int argc, char* argv[]);
};
}  // namespace request_generator

#endif  // REQUEST_GENERATOR_SRC_CONFIG_H_
