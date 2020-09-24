/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "../../common/include/cxxopts.hpp"
#include "../../common/include/json.hpp"
#include "../include/match_engine_stub_grpc.h"
#include "../include/order_manager.h"
#include "../include/order_store_influxdb.h"

using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

template <typename Out>
void split(const std::string& s, char delimiter, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delimiter)) {
    *result++ = item;
  }
}

void RunServer(const std::string& order_manager_address,
               const DatabaseConfig& database_config,
               const std::string& match_engine_main_address,
               const std::vector<std::string>& match_engine_request_addresses,
               const std::string& test_mode_is_open) {
  assert(!order_manager_address.empty());
  assert(!match_engine_main_address.empty());

  std::shared_ptr<Channel> main_channel;
  std::vector<std::shared_ptr<Channel>> request_channels;

  main_channel = grpc::CreateChannel(match_engine_main_address,
                                     grpc::InsecureChannelCredentials());

  if (!match_engine_request_addresses.empty()) {
    for (const auto& address : match_engine_request_addresses) {
      request_channels.push_back(
          grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    }
  }

  auto order_store = std::make_shared<OrderStoreInfluxDB>(database_config);

  auto match_engine_stub =
      std::make_shared<MatchEngineStubGrpc>(main_channel, request_channels);

  OrderManagerService service(order_store, match_engine_stub);
  nlohmann::json record_config;
  std::ifstream input("order_manager_record_config.json");
  input >> record_config;
  service.SetRecordTimeInterval(
      record_config["record_time_interval_in_seconds"]);
  service.SetLatencyAverageWarning(
      record_config["latency_average_warning_in_milliseconds"]);
  if (test_mode_is_open != "0") {
    service.SetTestModeIsOpen(true);
  }

  ServerBuilder builder;
  builder.AddListeningPort(order_manager_address,
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << order_manager_address << std::endl;
  server->Wait();
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("Order Manager", "Managing orders");
  options.add_options()("a,service_address", "Order manager service address",
                        cxxopts::value<std::string>())(
      "d,database_address",
      "Database address which is used by the order manager",
      cxxopts::value<std::string>())("m,match_engine_main_address",
                                     "Match engine main address",
                                     cxxopts::value<std::string>())(
      "r,match_engine_request_addresses", "Match engine request only addresses",
      cxxopts::value<std::string>())("h,help", "print usage")(
      "l,log_level", R"(Log level, "info" or "debug", default: info)",
      cxxopts::value<std::string>())(
      "f,log_file", "Log file location, default: /tmp/order_manager.log",
      cxxopts::value<std::string>())("t,test_mode_is_open",
                                     "Open test mode or not",
                                     cxxopts::value<std::string>())(
      "db_user", "InfluxDB username", cxxopts::value<std::string>())(
      "db_password", "InfluxDB password", cxxopts::value<std::string>())(
      "db_name",
      "Intended InfluxDB database name to use, default: order_manager",
      cxxopts::value<std::string>())(
      "db_measurement",
      "Intended InfluxDB measurement name to use, default: order",
      cxxopts::value<std::string>());

  auto result = options.parse(argc, argv);
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  if (!result.count("service_address") || !result.count("database_address") ||
      !result.count("match_engine_main_address")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  DatabaseConfig database_config;
  std::string order_manager_address =
      result["service_address"].as<std::string>();
  std::vector<std::string> influxdb_address_in_string;
  std::string match_engine_main_address =
      result["match_engine_main_address"].as<std::string>();
  std::vector<std::string> match_engine_request_addresses;
  std::string test_mode_is_open = result["test_mode_is_open"].as<std::string>();

  split(result["database_address"].as<std::string>(), ':',
        std::back_inserter(influxdb_address_in_string));
  database_config.db_address = influxdb_address_in_string[0];
  database_config.db_port = std::stoi(influxdb_address_in_string[1]);

  if (result.count("db_user")) {
    database_config.db_user = result["db_user"].as<std::string>();
  }
  if (result.count("db_password")) {
    database_config.db_password = result["db_password"].as<std::string>();
  }

  if (result.count("db_name")) {
    database_config.db_name = result["db_name"].as<std::string>();
  }
  if (result.count("db_measurement")) {
    database_config.db_measurement = result["db_measurement"].as<std::string>();
  }

  if (result.count("match_engine_request_addresses")) {
    split(result["match_engine_request_addresses"].as<std::string>(), ',',
          std::back_inserter(match_engine_request_addresses));
  }

  cdcf::CDCFConfig cdcf_config;
  if (result.count("log_level")) {
    cdcf_config.log_level_ = result["log_level"].as<std::string>();
  }
  if (cdcf_config.log_level_ != "debug") {
    cdcf_config.log_no_display_filename_and_line_number_ = true;
  }
  if (result.count("log_file")) {
    cdcf_config.log_file_ = result["log_file"].as<std::string>();
  } else {
    cdcf_config.log_file_ = "/tmp/order_manager.log";
  }
  cdcf::Logger::Init(cdcf_config);

  RunServer(order_manager_address, database_config, match_engine_main_address,
            match_engine_request_addresses, test_mode_is_open);

  return 0;
}
