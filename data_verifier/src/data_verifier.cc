/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */
#include "../include/data_verifier.h"

#include <cdcf/logger.h>

#include <thread>

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
    CDCF_LOGGER_INFO("start compare ordered data sources");
    ret = VerifyEqualityForOrderedDataSource(data_source_entry_number, 0);
  } else {
    CDCF_LOGGER_INFO("start compare unordered data sources");
    ret = VerifyEqualityForUnorderedDataSource(data_source_entry_number, 0);
  }

  return ret;
}

bool DataVerifier::VerifyEqualityForOrderedDataSource(int limit, int offset) {
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
