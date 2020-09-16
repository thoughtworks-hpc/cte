/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */
#include "../include/data_source_influxdb.h"

#include <cdcf/logger.h>
#include <fmt/core.h>

using json = nlohmann::json;

int DataSourceInfluxDB::GetDataEntryNumber() {
  int ret;
  std::string resp;
  std::string data;
  std::string sql = BuildGetDataEntryNumberQuery();
  ret = influxdb_cpp::query(data, sql, si_);
  if (0 == ret) {
    CDCF_LOGGER_DEBUG("query db success");
  } else {
    CDCF_LOGGER_ERROR("query db failed ret: {}", ret);
  }

  int count = 0;
  try {
    json j = json::parse(data);

    json j_results = j["results"].get<json>()[0];
    json j_series = j_results["series"].get<json>()[0];
    json j_values = j_series["values"].get<json>();
    count = j_values[0][1].get<int>();
  } catch (const std::exception& e) {
    CDCF_LOGGER_ERROR("get data entry number error:  {}", e.what());
    count = 0;
  }

  return count;
}

void DataSourceInfluxDB::GetDataEntries(int limit, int offset,
                                        std::string& data) {
  int ret;
  std::string resp;
  std::string sql = BuildGetDataEntriesQuery(limit, offset);
  ret = influxdb_cpp::query(data, sql, si_);
  if (0 == ret) {
    CDCF_LOGGER_DEBUG("query db success");
  } else {
    CDCF_LOGGER_ERROR("query db failed ret: {}", ret);
  }
}

std::vector<std::string> DataSourceInfluxDB::GetDataEntries(int limit,
                                                            int offset) {
  std::string data;
  GetDataEntries(limit, offset, data);

  return Algorithm::ExtractValuesElementFromJsonString(data).value_or(
      std::vector<std::string>());
}

std::optional<std::vector<std::string>>
DataSourceInfluxDB::Algorithm::ExtractValuesElementFromJsonString(
    const std::string& data) {
  std::vector<std::string> data_entries;
  std::optional<std::vector<std::string>> element;

  json j_source = json::parse(data);
  json j_source_values;

  if (Algorithm::ExtractValuesJsonArray(data, j_source_values) != 0) {
    return std::nullopt;
  }

  int j_src_size = j_source_values.size();
  if (j_src_size <= 0) {
    return std::nullopt;
  }

  for (int i = 0; i < j_src_size; ++i) {
    json j_element = j_source_values[i].get<json>();

    data_entries.push_back(j_element.dump());
  }
  element = data_entries;

  return element;
}

std::string DataSourceInfluxDB::GetQueryResult(const std::string& sql) {
  std::string resp;
  int ret = influxdb_cpp::query(resp, sql, si_);
  if (0 == ret) {
    CDCF_LOGGER_DEBUG("query db success");
  } else {
    CDCF_LOGGER_ERROR("query db failed ret: {} and sql: {}", ret, sql);
  }
  return resp;
}

bool DataSourceInfluxDB::IsEmptyQueryResult(const std::string& result) {
  return result == "{\"results\":[{\"statement_id\":0}]}\n";
}

//std::function<bool(const std::string& source, const std::string& target)>
//DataSourceInfluxDB::GetCompareFunction() {
//  return [](const std::string& source, const std::string& target) {
//    return Algorithm::CompareTradeJson(source, target);
//  };
//}

std::string DataSourceInfluxDB::BuildGetDataEntriesQuery(int limit,
                                                         int offset) {
  std::string sql = "select * from ";
  sql += "\"";
  sql += measurement_;
  sql += "\"";
  sql += " limit " + std::to_string(limit);
  sql += " offset " + std::to_string(offset);

  CDCF_LOGGER_DEBUG("BuildGetDataEntriesQuery SQL: {}", sql);
  return sql;
}

std::string DataSourceInfluxDB::BuildGetDataEntryNumberQuery() {
  std::string sql = "select count(trade_id) from ";
  sql += "\"";
  sql += measurement_;
  sql += "\"";

  CDCF_LOGGER_DEBUG("BuildGetDataEntryNumberQuery SQL: {}", sql);
  return sql;
}

std::string DataSourceInfluxDB::BuildFindIfEntryExistsQuery(
    const std::string& amount, const std::string& buy_order_id,
    const std::string& buy_user_id, const std::string& price,
    const std::string& sell_order_id, const std::string& sell_user_id,
    const std::string& symbol_id) {
  std::string sql =
      fmt::format(R"(select "symbol_id" from "{}" where "amount" = '{}'
and "buy_order_id" = '{}'
and "buy_user_id" = '{}'
and "price" = '{}'
and "sell_order_id" = '{}'
and "sell_user_id" = '{}'
and "symbol_id" = '{}')",
                  measurement_, amount, buy_order_id, buy_user_id, price,
                  sell_order_id, sell_user_id, symbol_id);

  CDCF_LOGGER_DEBUG("BuildFindIfEntryExistsQuery SQL: {}", sql);
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
    CDCF_LOGGER_ERROR("compare trade json element error:  {}", e.what());
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
    CDCF_LOGGER_ERROR("extract values json array error:  {}", e.what());
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
//      CDCF_LOGGER_INFO("trade inconsistent between {} and {}",
//                       j_trade_element_source.dump(),
//                       j_trade_element_target.dump());
      return false;
    }
  }

  return true;
}

bool DataSourceInfluxDB::FindIfDataEntryExists(const std::string& entry) {
  j_entry_ = json::parse(entry);

  //  std::string amount;
  //  std::string buy_order_id;
  //  std::string buy_user_id;
  //  std::string price;
  //  std::string sell_order_id;
  //  std::string sell_user_id;
  //  std::string symbol_id;

  try {
    amount_ = j_entry_[1].get<std::string>();
    buy_order_id_ = j_entry_[2].get<std::string>();
    buy_user_id_ = j_entry_[3].get<std::string>();
    price_ = j_entry_[4].get<std::string>();
    sell_order_id_ = j_entry_[5].get<std::string>();
    sell_user_id_ = j_entry_[6].get<std::string>();
    symbol_id_ = j_entry_[7].get<std::string>();
  } catch (const std::exception& e) {
    CDCF_LOGGER_ERROR("FindIfJsonExists error:  {}", e.what());
    return false;
  }

  std::string sql(std::move(
      BuildFindIfEntryExistsQuery(amount_, buy_order_id_, buy_user_id_, price_,
                                  sell_order_id_, sell_user_id_, symbol_id_)));

  std::string resp(std::move(GetQueryResult(sql)));

  if (IsEmptyQueryResult(resp)) {
    CDCF_LOGGER_ERROR("QueryResult :  {}", resp);
    return false;
  } else {
    return true;
  }
}

bool DataSourceInfluxDB::FindIfJsonExists(
    const json& j_source, const std::shared_ptr<DataSource>& data_source) {
  std::string amount;
  std::string buy_order_id;
  std::string buy_user_id;
  std::string price;
  std::string sell_order_id;
  std::string sell_user_id;
  std::string symbol_id;

  try {
    amount = j_source[1].get<std::string>();
    buy_order_id = j_source[2].get<std::string>();
    buy_user_id = j_source[3].get<std::string>();
    price = j_source[4].get<std::string>();
    sell_order_id = j_source[5].get<std::string>();
    sell_user_id = j_source[6].get<std::string>();
    symbol_id = j_source[7].get<std::string>();
  } catch (const std::exception& e) {
    CDCF_LOGGER_ERROR("FindIfJsonExists error:  {}", e.what());
    return false;
  }

  std::string sql =
      BuildFindIfEntryExistsQuery(amount, buy_order_id, buy_user_id, price,
                                  sell_order_id, sell_user_id, symbol_id);

  std::string resp = GetQueryResult(sql);

  return !IsEmptyQueryResult(resp);
}

bool DataSourceInfluxDB::CompareDataEntry(const std::string& source,
                                          const std::string& target) {
  return Algorithm::CompareTradeJson(source, target);
}
