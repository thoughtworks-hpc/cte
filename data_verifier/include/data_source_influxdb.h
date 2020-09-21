/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_SOURCE_INFLUXDB_H_
#define DATA_VERIFIER_INCLUDE_DATA_SOURCE_INFLUXDB_H_

#include <string>
#include <utility>
#include <vector>

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"
#include "./data_source.h"

class DataSourceInfluxDB : public DataSource {
 public:
  DataSourceInfluxDB(influxdb_cpp::server_info si, std::string measurement)
      : si_(std::move(si)), measurement_(std::move(measurement)) {}

  int GetDataEntryNumber() override;

  std::vector<std::string> GetDataEntries(int limit, int offset) override;

  bool CompareDataEntry(const std::string& source,
                        const std::string& target) override;

  bool FindIfDataEntryExists(const std::string& entry) override;

  struct Algorithm {
    static bool CompareTradeJson(const std::string& source,
                                 const std::string& target);

    static int ExtractValuesJsonArray(const std::string& data,
                                      nlohmann::json& values_array);
    static bool CompareTradeJsonElement(const nlohmann::json& j_source,
                                        const nlohmann::json& j_target);
    static std::optional<std::vector<std::string>>
    ExtractValuesElementFromJsonString(const std::string& j);
  };

 private:
  void GetDataEntries(int limit, int offset, std::string& data);
  std::string GetQueryResult(const std::string& sql);
  bool IsEmptyQueryResult(const std::string& result);

  std::string BuildGetDataEntriesQuery(int limit, int offset);
  std::string BuildGetDataEntryNumberQuery();

  influxdb_cpp::server_info si_;
  std::string measurement_;
  std::string BuildFindIfEntryExistsQuery(const std::string& amount,
                                          const std::string& buy_order_id,
                                          const std::string& buy_user_id,
                                          const std::string& price,
                                          const std::string& sell_order_id,
                                          const std::string& sell_user_id,
                                          const std::string& symbol_id);
  std::string amount_;
  std::string buy_order_id_;
  std::string buy_user_id_;
  std::string price_;
  std::string sell_order_id_;
  std::string sell_user_id_;
  std::string symbol_id_;
  nlohmann::json j_entry_;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_SOURCE_INFLUXDB_H_
