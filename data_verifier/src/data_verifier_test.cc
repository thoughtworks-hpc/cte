/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/data_verifier.h"

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "../include/data_source_influxdb.h"
#include "../include/data_source_mock.h"

using nlohmann::json;

TEST(DataVerifier,
     should_retrieve_all_data_from_ordered_data_source_when_verify_equality) {
  auto data_source_a = std::make_shared<DataSourceMock>();
  auto data_source_b = std::make_shared<DataSourceMock>();

  DataVerifier data_verifier(data_source_a, data_source_b, true);
  data_verifier.VerifyEquality();

  EXPECT_TRUE(data_source_a->IfGotAllDataEntries());
  EXPECT_TRUE(data_source_b->IfGotAllDataEntries());
}

TEST(
    DataVerifier,
    should_retrieve_all_data_from_not_orderd_data_source_when_verify_equality) {
  auto data_source_a = std::make_shared<DataSourceMock>();
  auto data_source_b = std::make_shared<DataSourceMock>();

  DataVerifier data_verifier(data_source_a, data_source_b, false);
  data_verifier.VerifyEquality();

  EXPECT_TRUE(data_source_a->IfGotAllDataEntries());
}

TEST(Algorithm, should_return_correct_vector_when_extracting_from_json_string) {
  std::string json_string =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7979"]]}]}]})";
  std::vector<std::string> expected;
  expected.push_back(
      R"([1599515557929,"O","8","1","3","2","4","15","be9d5d57-a62f-4733-9a80-f5cae99e7979"])");
  EXPECT_EQ(DataSourceInfluxDB::Algorithm::ExtractValuesElementFromJsonString(
                json_string)
                .value(),
            expected);
}

TEST(Algorithm, should_return_correct_map_when_extracting_from_json_string) {
  std::string json_string =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1",1599515557929,"3","2","4","1","be9d5d57-a62f-4733-9a80-f5cae99e7979"],
[1599515553543,"1","4","1",1599515557929,"3","33","4","2","be9d5d57-a62f-4733-9a80-f5cae99e5432"]]}]}]})";
  std::unordered_map<std::string, std::vector<std::string>> extracted;
  std::unordered_map<std::string, std::vector<std::string>> expected;
  expected["1"].push_back(
      R"([1599515557929,"O","8","1",1599515557929,"3","2","4","1","be9d5d57-a62f-4733-9a80-f5cae99e7979"])");
  expected["2"].push_back(
      R"([1599515553543,"1","4","1",1599515557929,"3","33","4","2","be9d5d57-a62f-4733-9a80-f5cae99e5432"])");

  extracted =
      DataSourceInfluxDB::Algorithm::ExtractValuesElementFromJsonStringBySymbol(
          json_string)
          .value();
  EXPECT_EQ(extracted.size(), 2);
  EXPECT_EQ(extracted, expected);
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
      R"([1599515557929,"O","8","1","3","2","4",1599515557929,"15","be9d5d57-a62f-4733-9a80-f5cae99e7979"])"_json;
  json j2 =
      R"([1599515557272,"O","8","1","3","2","4",1599515557928,"15","be9d5d57-a62f-4733-9a80-f5cae99e7727"])"_json;

  EXPECT_TRUE(DataSourceInfluxDB::Algorithm::CompareTradeJsonElement(j1, j2));
}

TEST(DataVerifier, should_return_true_when_comparing_equal_trade_json) {
  std::string json_string_src =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1","3","2","4",1599515557929,"15","be9d5d57-a62f-4733-9a80-f5cae99e7979"],
[1599515553543,"1","4","1","3","33","4",1599515557929,"77","be9d5d57-a62f-4733-9a80-f5cae99e5432"]]}]}]})";
  std::string json_string_tar =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557272,"O","8","1","3","2","4",1599515557272,"15","be9d5d57-a62f-4733-9a80-f5cae99e7727"],
[1599515553345,"1","4","1","3","33","4",1599515557272,"77","be9d5d57-a62f-4733-9a80-f5cae99e5632"]]}]}]})";

  EXPECT_TRUE(DataSourceInfluxDB::Algorithm::CompareTradeJson(json_string_src,
                                                              json_string_tar));
}

TEST(DataVerifier, should_return_false_when_comparing_unequal_trade_json) {
  std::string json_string_src =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557929,"O","8","1","3","2","4",1599515557929,"27","be9d5d57-a62f-4733-9a80-f5cae99e7979"],
[1599515553543,"1","4","1","3","33","4",1599515557929,"77","be9d5d57-a62f-4733-9a80-f5cae99e5432"]]}]}]})";
  std::string json_string_tar =
      R"({"results":[{"statement_id":0,"series":[{"name":"trades","columns":["time","amount","buy_trade_id",
"buy_user_id","price","sell_trade_id","sell_user_id","symbol_id","trade_id"],
"values":[[1599515557272,"O","8","1","3","2","4",1599515557272,"15","be9d5d57-a62f-4733-9a80-f5cae99e7727"],
[1599515553345,"1","4","1","3","33","4",1599515553345,"77","be9d5d57-a62f-4733-9a80-f5cae99e5632"]]}]}]})";

  EXPECT_FALSE(DataSourceInfluxDB::Algorithm::CompareTradeJson(
      json_string_src, json_string_tar));
}
