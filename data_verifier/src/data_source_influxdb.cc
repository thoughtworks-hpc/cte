/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/data_source_influxdb.h"

#include <iostream>

using json = nlohmann::json;

void DataSourceInfluxDB::GetDataEntries(int limit, int offset,
                                        std::string& data) {
  int ret;
  std::string resp;
  std::string sql = BuildQuerySql(limit, offset);
  ret = influxdb_cpp::query(data, sql, si_);
  if (0 == ret) {
    std::cout << "query db success" << std::endl;
  } else {
    std::cout << "query db failed ret:" << ret << std::endl;
  }
}

std::function<bool(const std::string& source, const std::string& target)>
DataSourceInfluxDB::GetCompareFunction() {
  return [](const std::string& source, const std::string& target) {
    return Algorithm::CompareTradeJson(source, target);
  };
}

std::string DataSourceInfluxDB::BuildQuerySql(int limit, int offset) {
  std::string sql = "select * from ";
  sql += "\"";
  sql += measurement_;
  sql += "\"";
  sql += " limit " + std::to_string(limit);
  sql += " offset " + std::to_string(offset);

  std::cout << "sql: " << sql << std::endl;
  return sql;
}

bool DataSourceInfluxDB::Algorithm::CompareTradeJsonElement(
    const json& j_source, const json& j_target) {
  int j_src_size = j_source.size();
  int j_tar_size = j_target.size();
  if (j_src_size != j_tar_size) {
    return false;
  }

  try {
    // amount
    if (j_source[1].get<std::string>() != j_target[1].get<std::string>()) {
      return false;
    }
    // buy_trade_id
    if (j_source[2].get<std::string>() != j_target[2].get<std::string>()) {
      return false;
    }
    // buy_user_id
    if (j_source[3].get<std::string>() != j_target[3].get<std::string>()) {
      return false;
    }
    // price
    if (j_source[4].get<std::string>() != j_target[4].get<std::string>()) {
      return false;
    }
    // sell_trade_id
    if (j_source[5].get<std::string>() != j_target[5].get<std::string>()) {
      return false;
    }
    // sell_user_id
    if (j_source[6].get<std::string>() != j_target[6].get<std::string>()) {
      return false;
    }
    // symbol_id
    if (j_source[7].get<std::string>() != j_target[7].get<std::string>()) {
      return false;
    }
  } catch (const std::exception& e) {
    std::cout << "compare data entry error: " << e.what() << std::endl;
    return false;
  }
  return true;
}

int DataSourceInfluxDB::Algorithm::ExtractValuesJsonArray(
    const std::string& data, json& values_array) {
  try {
    json j = json::parse(data);
    json j_results = j["results"].get<json>()[0];
    json j_series = j_results["series"].get<json>()[0];
    values_array = j_series["values"].get<json>();
  } catch (const std::exception& e) {
    std::cout << "compare data entry error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

bool DataSourceInfluxDB::Algorithm::CompareTradeJson(
    const std::string& source, const std::string& target) {
  json j_source = json::parse(source);
  json j_target = json::parse(target);
  json j_source_values;
  json j_target_values;

  if (Algorithm::ExtractValuesJsonArray(source, j_source_values) != 0) {
    return false;
  }
  if (Algorithm::ExtractValuesJsonArray(target, j_target_values) != 0) {
    return false;
  }

  int j_src_size = j_source_values.size();
  int j_tar_size = j_target_values.size();
  if (j_src_size != j_tar_size) {
    return false;
  }

  for (int i = 0; i < j_src_size; ++i) {
    json j_trade_element_source = j_source_values[i].get<json>();
    json j_trade_element_target = j_target_values[i].get<json>();
    if (!CompareTradeJsonElement(j_trade_element_source,
                                 j_trade_element_target)) {
      return false;
    }
  }

  return true;
}
