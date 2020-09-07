/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <gtest/gtest.h>

#include "../include/data_source_influxdb.h"

//TEST(DataVerifier, test_json_parsing) {
//  influxdb_cpp::server_info si("127.0.0.1", 8086, "order_manager", "", "");
//  DataSourceInfluxDB db(si, "order");
//
//  std::string data;
//  db.GetDataEntries(5, 1, data);
//
//  using json = nlohmann::json;
//  json j = json::parse(data);
//
//  json j_results = j["results"].get<json>()[0];
//  json j_series = j_results["series"].get<json>()[0];
//  json j_values = j_series["values"].get<json>();
//
//  std::cout << "data: " << data << std::endl;
//  std::cout << "j_results: " << j_results << std::endl;
//  std::cout << "j_series: " << j_series << std::endl;
//  std::cout << "j_values: " << j_values << std::endl;
//
//  for (auto& [key, value] : j_values.items()) {
//    std::cout << key << " : " << value << "\n";
//  }
//}