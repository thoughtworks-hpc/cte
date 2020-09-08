/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <gtest/gtest.h>

#include "../include/data_verifier.h"
#include "../include/data_source_influxdb.h"
#include "../include/data_source_mock.h"

using nlohmann::json;

// TEST(DataVerifier, test_json_parsing) {
//  influxdb_cpp::server_info si("127.0.0.1", 8086, "akka", "", "");
//  DataSourceInfluxDB db(si, "trades");

//  std::string data;
//  db.GetDataEntries(5, 1, data);
//  EXPECT_EQ(db.GetDataEntryNumber(), 3);
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

TEST(DataVerifier, should_retrieve_all_data_from_data_source_when_verify_equality) {
  auto data_source_a = std::make_shared<DataSourceMock>();
  auto data_source_b = std::make_shared<DataSourceMock>();

  DataVerifier data_verifier(data_source_a, data_source_b);
  data_verifier.VerifyEquality();

  EXPECT_TRUE(data_source_a->IfGotAllDataEntries());
  EXPECT_TRUE(data_source_b->IfGotAllDataEntries());
}

TEST(DataVerifier, should_return_values_json_array_when_extracting) {
  std::string json_string =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7979"]]}]}]})";
  json j;
  DataSourceInfluxDB::Algorithm::ExtractValuesJsonArray(json_string, j);
  EXPECT_EQ(j.dump(),
            "[[1599515557929,\"O\",\"8\",\"1\",\"3\",\"2\",\"4\",\"15\","
            "\"be9d5d57-a62f-4733-9a80-f5cae99e7979\"]]");
}

TEST(DataVerifier, should_return_true_when_comparing_equal_trade_json_element) {
  json j1 =
      R"([1599515557929,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7979"])"_json;
  json j2 =
      R"([1599515557272,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7727"])"_json;

  EXPECT_TRUE(DataSourceInfluxDB::Algorithm::CompareTradeJsonElement(j1, j2));
}

TEST(DataVerifier, should_return_true_when_comparing_equal_trade_json) {
  std::string json_string_src =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7979"],
[1599515553543,"1","4","1","3","33","4","77","be9d5d57-a62f-4733-9a80-f5cae99e5432"]]}]}]})";
  std::string json_string_tar =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557272,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7727"],
[1599515553345,"1","4","1","3","33","4","77","be9d5d57-a62f-4733-9a80-f5cae99e5632"]]}]}]})";

  EXPECT_TRUE(DataSourceInfluxDB::Algorithm::CompareTradeJson(json_string_src,
                                                              json_string_tar));
}

TEST(DataVerifier, should_return_false_when_comparing_unequal_trade_json) {
  std::string json_string_src =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1","3","2","4","27","be9d5d57-a62f-4733-9a80-f5cae99e7979"],
[1599515553543,"1","4","1","3","33","4","77","be9d5d57-a62f-4733-9a80-f5cae99e5432"]]}]}]})";
  std::string json_string_tar =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557272,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7727"],
[1599515553345,"1","4","1","3","33","4","77","be9d5d57-a62f-4733-9a80-f5cae99e5632"]]}]}]})";

  EXPECT_FALSE(DataSourceInfluxDB::Algorithm::CompareTradeJson(
      json_string_src, json_string_tar));
}
