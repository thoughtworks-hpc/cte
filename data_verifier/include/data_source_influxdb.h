/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_DATA_SOURCE_INFLUXDB_H
#define CTE_DATA_SOURCE_INFLUXDB_H

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"
#include "./data_source.h"

class DataSourceInfluxDB : public DataSource {
 public:
  DataSourceInfluxDB(influxdb_cpp::server_info si,
                     const std::string& measurement)
      : si_(si), measurement_(measurement) {}

  void GetDataEntries(int limit, int offset, std::string& data);

 private:
  std::string BuildQuerySql(int limit, int offset);
  bool CompareDataEntries(const std::string& source, const std::string& target);
  bool CompareDataEntry(const nlohmann::json& source,
                        const nlohmann::json& target);
  nlohmann::json&& GetDataEntries(const std::string& data);

  influxdb_cpp::server_info si_;
  std::string measurement_;
};

#endif  // CTE_DATA_SOURCE_INFLUXDB_H
