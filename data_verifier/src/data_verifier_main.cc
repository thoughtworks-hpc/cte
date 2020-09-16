/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <cdcf/logger.h>

#include <fstream>
#include <iostream>

#include "../../common/include/json.hpp"
#include "../include/data_source_influxdb.h"
#include "../include/data_verifier.h"

using nlohmann::json;

int ParserConfigJsonForDatabaseServerInfo(
    const json& config, influxdb_cpp::server_info& server_info_a,
    influxdb_cpp::server_info& server_info_b, std::string& server_a_measurement,
    std::string& server_b_measurement) {
  try {
    json server = config["server"].get<json>();
    json server_a = server[0].get<json>();
    json server_b = server[1].get<json>();

    server_info_a.host_ = server_a["host"].get<std::string>();
    server_info_a.port_ = server_a["port"].get<int>();
    server_info_a.db_ = server_a["database"].get<std::string>();
    server_info_a.usr_ = server_a["user"].get<std::string>();
    server_info_a.pwd_ = server_a["password"].get<std::string>();
    server_a_measurement = server_a["measurement"].get<std::string>();

    server_info_b.host_ = server_b["host"].get<std::string>();
    server_info_b.port_ = server_b["port"].get<int>();
    server_info_b.db_ = server_b["database"].get<std::string>();
    server_info_b.usr_ = server_b["user"].get<std::string>();
    server_info_b.pwd_ = server_b["password"].get<std::string>();
    server_b_measurement = server_b["measurement"].get<std::string>();
  } catch (const std::exception& e) {
    std::cout << "ParserConfigJsonForDatabaseServerInfo error: " << e.what()
              << std::endl;
    return 1;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  nlohmann::json config;

  if (argc == 1) {
    std::ifstream input("data_verifier_config.json");
    input >> config;
  } else if (argc == 2) {
    std::ifstream in(argv[1]);
    in >> config;
  } else {
    std::cout << "Usage: data_verifier [data_verifier_config.json]"
              << std::endl;
    return 1;
  }

  cdcf::CDCFConfig cdcf_config;
  cdcf_config.log_level_ = "info";
  cdcf_config.log_file_ = "/tmp/data_verifier.log";
  cdcf_config.log_no_display_filename_and_line_number_ = true;
  cdcf::Logger::Init(cdcf_config);

  influxdb_cpp::server_info server_info_a("127.0.0.1", 8086);
  influxdb_cpp::server_info server_info_b("127.0.0.1", 8086);
  std::string measurement_a;
  std::string measurement_b;
  int ret = 0;
  ret = ParserConfigJsonForDatabaseServerInfo(
      config, server_info_a, server_info_b, measurement_a, measurement_b);
  if (ret != 0) {
    std::cout << "parse config json failed" << std::endl;
  }

  auto data_source_a =
      std::make_shared<DataSourceInfluxDB>(server_info_a, measurement_a);
  auto data_source_b =
      std::make_shared<DataSourceInfluxDB>(server_info_b, measurement_b);

  DataVerifier data_verifier(data_source_a, data_source_b);

  if (data_verifier.VerifyEquality()) {
    CDCF_LOGGER_INFO("trade data matches between 2 data sources");
  } else {
    CDCF_LOGGER_INFO("trade data doesn't match between 2 data sources");
  }
}
