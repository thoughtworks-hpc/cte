/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */
#include "../include/data_verifier.h"

#include <cdcf/logger.h>

#include <thread>

#include "../../common/include/json.hpp"

bool DataVerifier::VerifyEquality() {
  std::string data_a;
  std::string data_b;

  int data_source_a_number = data_source_a_->GetDataEntryNumber();
  int data_source_b_number = data_source_b_->GetDataEntryNumber();

  if (data_source_a_number != data_source_b_number) {
    CDCF_LOGGER_INFO(
        "trade data entry number inconsistent between {} and {} from 2 data "
        "sources",
        data_source_a_number, data_source_b_number);
  }
  int data_source_entry_number =
      std::min(data_source_a_number, data_source_b_number);

  if (compare_entire_data_source_in_one_turn) {
    number_of_entries_to_compare_each_turn_ = data_source_entry_number;
  }

  int ret;
  if (is_ordered_data_sources_) {
    if (is_ordered_by_symbol_) {
      CDCF_LOGGER_INFO("start to compare ordered by symbol data sources");
      ret = VerifyEqualityForOrderedBySymbolDataSource(data_source_a_number,
                                                       data_source_b_number, 0);
    } else {
      CDCF_LOGGER_INFO("start to compare fully ordered data sources");
      ret =
          VerifyEqualityForFullyOrderedDataSource(data_source_entry_number, 0);
    }
  } else {
    CDCF_LOGGER_INFO("start to compare unordered data sources");
    ret = VerifyEqualityForUnorderedDataSource(data_source_entry_number, 0);
  }

  return ret;
}

bool DataVerifier::VerifyEqualityForFullyOrderedDataSource(int limit,
                                                           int offset) {
  int data_source_number_remaining = limit;
  int data_source_number_to_retrieve =
      limit > number_of_entries_to_compare_each_turn_
          ? number_of_entries_to_compare_each_turn_
          : limit;

  CDCF_LOGGER_INFO("{} entries to compare", limit);
  bool inconsistency_found = false;

  while (data_source_number_remaining > 0) {
    auto data_entries_a =
        data_source_a_->GetDataEntries(data_source_number_to_retrieve, offset);
    auto data_entries_b =
        data_source_b_->GetDataEntries(data_source_number_to_retrieve, offset);

    for (int i = 0; i < data_entries_a.size(); ++i) {
      if (!data_source_a_->CompareDataEntry(data_entries_a[i],
                                            data_entries_b[i])) {
        CDCF_LOGGER_ERROR(
            "trade data inconsistent between {} and {} from 2 data sources",
            data_entries_a[i], data_entries_b[i]);
        inconsistency_found = true;
      }
    }

    data_source_number_remaining -= data_source_number_to_retrieve;
    offset += data_source_number_to_retrieve;
    CDCF_LOGGER_INFO(
        "{} entries compared with {} remaining", data_source_number_to_retrieve,
        data_source_number_remaining > 0 ? data_source_number_remaining : 0);
  }
  return !inconsistency_found;
}

bool DataVerifier::VerifyEqualityForOrderedBySymbolDataSource(
    int data_source_a_limit, int data_source_b_limit, int offset) {
  CDCF_LOGGER_INFO("{} entries to compare",
                   std::max(data_source_a_limit, data_source_b_limit));
  bool inconsistency_found = false;

  auto data_entries_map_a =
      data_source_a_->GetDataEntriesBySymbol(data_source_a_limit, offset);
  auto data_entries_map_b =
      data_source_b_->GetDataEntriesBySymbol(data_source_b_limit, offset);

  CDCF_LOGGER_INFO("finish loading data from data sources");

  std::string symbol_id;
  for (const auto& pair_a : data_entries_map_a) {
    symbol_id = pair_a.first;
    CDCF_LOGGER_INFO("comparison of symbol {} starts", symbol_id);

    if (data_entries_map_b.find(symbol_id) == data_entries_map_b.end()) {
      CDCF_LOGGER_ERROR(
          "symbol {} from data source {} not found in data source {}}",
          symbol_id, data_source_a_->GetDataSourceName(),
          data_source_b_->GetDataSourceName());
      inconsistency_found = true;
      continue;
    }

    auto& data_entries_a = data_entries_map_a[symbol_id];
    auto& data_entries_b = data_entries_map_b[symbol_id];

    if (data_entries_a.size() != data_entries_b.size()) {
      CDCF_LOGGER_ERROR(
          "trade data entry number inconsistent between {} and {} from two "
          "data "
          "sources for symbol {}",
          data_entries_a.size(), data_entries_b.size(), symbol_id);
    }

    std::map<std::string, std::string> unmatched_data_entries_a;
    std::map<std::string, std::string> unmatched_data_entries_b;

    int i;
    for (i = 0; i < std::min(data_entries_a.size(), data_entries_b.size());
         ++i) {
      if (!data_source_a_->CompareDataEntry(data_entries_a[i],
                                            data_entries_b[i])) {
        nlohmann::json json_entry_a = nlohmann::json::parse(data_entries_a[i]);
        nlohmann::json json_entry_b = nlohmann::json::parse(data_entries_b[i]);

        std::string amount_a = json_entry_a[1].get<std::string>();
        std::string amount_b = json_entry_b[1].get<std::string>();
        std::string buy_user_id_a = json_entry_a[3].get<std::string>();
        std::string buy_user_id_b = json_entry_b[3].get<std::string>();
        std::string price_a = json_entry_a[4].get<std::string>();
        std::string price_b = json_entry_b[4].get<std::string>();
        std::string sell_user_id_a = json_entry_a[6].get<std::string>();
        std::string sell_user_id_b = json_entry_b[6].get<std::string>();

        unmatched_data_entries_a[amount_a + ":" + buy_user_id_a + ":" +
                                 price_a + ":" + sell_user_id_a] =
            data_entries_a[i];
        unmatched_data_entries_b[amount_b + ":" + buy_user_id_b + ":" +
                                 price_b + ":" + sell_user_id_b] =
            data_entries_b[i];
      }
    }

    if (!unmatched_data_entries_a.empty() ||
        !unmatched_data_entries_b.empty()) {
      auto itor_a = unmatched_data_entries_a.begin();
      while (itor_a != unmatched_data_entries_a.end()) {
        auto itor_b = unmatched_data_entries_b.find(itor_a->first);
        if (itor_b == unmatched_data_entries_b.end()) {
          inconsistency_found = true;
          itor_a++;
        } else {
          itor_a = unmatched_data_entries_a.erase(itor_a);
          unmatched_data_entries_b.erase(itor_b);
        }
      }

      if (!unmatched_data_entries_a.empty()) {
        CDCF_LOGGER_ERROR("unmatched entries from data source {}:",
                          data_source_a_->GetDataSourceName());
        for (const auto& entry : unmatched_data_entries_a) {
          CDCF_LOGGER_DEBUG(
              "{}", data_source_a_->GetDataEntryDebugString(entry.second));
        }
      }
      if (!unmatched_data_entries_b.empty()) {
        CDCF_LOGGER_ERROR("unmatched entries from data source {}:",
                          data_source_b_->GetDataSourceName());
        for (const auto& entry : unmatched_data_entries_b) {
          CDCF_LOGGER_DEBUG(
              "{}", data_source_b_->GetDataEntryDebugString(entry.second));
        }
      }
    }

    if (i < data_entries_a.size()) {
      CDCF_LOGGER_ERROR("leftover entries from data source {}:",
                        data_source_a_->GetDataSourceName());
      for (int j = i; j < data_entries_a.size(); ++j) {
        CDCF_LOGGER_DEBUG(
            "{}", data_source_a_->GetDataEntryDebugString(data_entries_a[j]));
      }
    }

    if (i < data_entries_b.size()) {
      CDCF_LOGGER_ERROR("leftover entries from data source {}:",
                        data_source_b_->GetDataSourceName());
      for (int j = i; j < data_entries_b.size(); ++j) {
        CDCF_LOGGER_DEBUG(
            "{}", data_source_b_->GetDataEntryDebugString(data_entries_b[j]));
      }
    }

    CDCF_LOGGER_INFO("comparison of symbol {} finishes", symbol_id);
  }
  return !inconsistency_found;
}

bool DataVerifier::VerifyEqualityForUnorderedDataSource(int limit, int offset) {
  int data_source_number_remaining = limit;
  int data_source_number_to_retrieve =
      limit > number_of_entries_to_compare_each_turn_
          ? number_of_entries_to_compare_each_turn_
          : limit;

  CDCF_LOGGER_INFO("{} entries to compare", limit);
  bool inconsistency_found = false;

  while (data_source_number_remaining > 0) {
    auto data_entries =
        data_source_a_->GetDataEntries(data_source_number_to_retrieve, offset);

    for (const auto& data_entry : data_entries) {
      if (!data_source_b_->FindIfDataEntryExists(data_entry)) {
        CDCF_LOGGER_ERROR(
            "cannot find corresponding data entry {} from second data source",
            data_entry);
        inconsistency_found = true;
      }
    }

    data_source_number_remaining -= data_source_number_to_retrieve;
    offset += data_source_number_to_retrieve;
    CDCF_LOGGER_INFO(
        "{} entries compared with {} remaining", data_source_number_to_retrieve,
        data_source_number_remaining > 0 ? data_source_number_remaining : 0);
  }
  return !inconsistency_found;
}
