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

    std::function<bool(const std::string& source,
                       const std::string& target)>
    GetCompareFunction();

  struct Algorithm {
    static bool CompareTradeJson(const std::string& source, const std::string& target);

    static int ExtractValuesJsonArray(const std::string& data,
                                nlohmann::json& values_array);
    static bool CompareTradeJsonElement(const nlohmann::json& j_source,
                                        const nlohmann::json& j_target);
  };

 private:
  std::string BuildQuerySql(int limit, int offset);

  influxdb_cpp::server_info si_;
  std::string measurement_;
};

#endif  // CTE_DATA_SOURCE_INFLUXDB_H
