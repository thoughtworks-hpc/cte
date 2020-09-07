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

bool DataSourceInfluxDB::CompareDataEntries(const std::string& source,
                                            const std::string& target) {
  json j_src = GetDataEntries(source);
  json j_tar = GetDataEntries(target);

  int j_src_size = j_src.size();
  int j_tar_size = j_tar.size();
  if (j_src_size != j_tar_size) {
    return false;
  }

  for (int i = 0; i < j_src_size; ++i) {
    json entry_src =  j_src[i].get<json>();
    json entry_tar =  j_tar[i].get<json>();


  }

  return 0;
}

bool DataSourceInfluxDB::CompareDataEntry(const json& source,
                                          const json& target) {
  int j_src_size = source.size();
  int j_tar_size = target.size();
  if (j_src_size != j_tar_size) {
    return false;
  }

  for (int i = 1; i < j_src_size; ++i) {
  }

  return false;
}

nlohmann::json&& DataSourceInfluxDB::GetDataEntries(const std::string& data) {
  json j = json::parse(data);
  json j_results = j["results"].get<json>()[0];
  json j_series = j_results["series"].get<json>()[0];
  json j_values = j_series["values"].get<json>();

  return std::move(j_values);
}

