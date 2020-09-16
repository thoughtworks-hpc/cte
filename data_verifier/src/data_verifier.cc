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
    CDCF_LOGGER_INFO("trade data entry number inconsistent between {} and {}",
                     data_source_a_number, data_source_b_number);
  }
  int data_source_entry_number = data_source_a_number;

  CDCF_LOGGER_INFO("start");

  int ret;
  if (is_ordered_data_sources_) {
    ret = VerifyEqualityForOrderedDataSet(data_source_entry_number, 0);
  } else {
    ret = VerifyEqualityForRandomDataSet(data_source_entry_number, 0);
  }

  //  while (data_source_number_remaining > 0) {
  //    auto data_entries =
  //        data_source_a_->GetDataEntries(data_source_number_to_retrieve,
  //        offset);
  //
  //    for (const auto& data_entry : data_entries) {
  //      if (!data_source_b_->FindIfDataEntryExists(data_entry)) {
  //        CDCF_LOGGER_ERROR("cannot find corresponding data entry {}",
  //                          data_entry);
  //        return false;
  //      }
  //    }
  //
  //    data_source_number_remaining -= data_source_number_to_retrieve;
  //    offset += data_source_number_to_retrieve;
  //    CDCF_LOGGER_INFO("{} entries compared with {} remaining",
  //                     data_source_number_to_retrieve,
  //                     data_source_number_remaining);
  //  }

  CDCF_LOGGER_INFO("finish");

  return ret;
}

bool DataVerifier::VerifyEqualityForOrderedDataSet(int limit, int offset) {
  int data_source_number_remaining = limit;
  int data_source_number_to_retrieve =
      limit > number_of_entries_to_compare_each_turn_
          ? number_of_entries_to_compare_each_turn_
          : limit;

  CDCF_LOGGER_INFO("VerifyEqualityForOrderedDataSet with limit {} & offset {}",
                   limit, offset);

  while (data_source_number_remaining > 0) {
    auto data_entries_a =
        data_source_a_->GetDataEntries(data_source_number_to_retrieve, offset);
    auto data_entries_b =
        data_source_b_->GetDataEntries(data_source_number_to_retrieve, offset);

    for (int i = 0; i < data_entries_a.size(); ++i) {
      if (!data_source_a_->CompareDataEntry(data_entries_a[i],
                                            data_entries_b[i])) {
        CDCF_LOGGER_INFO("trade inconsistent between {} and {}",
                         data_entries_a[i], data_entries_b[i]);
        return false;
      }
    }

    for (const auto& data_entry : data_entries_a) {
      if (!data_source_b_->FindIfDataEntryExists(data_entry)) {
        CDCF_LOGGER_ERROR(
            "cannot find corresponding data entry {} from second data source",
            data_entry);
        return false;
      }
    }

    data_source_number_remaining -= data_source_number_to_retrieve;
    offset += data_source_number_to_retrieve;
    CDCF_LOGGER_INFO(
        "{} entries compared with {} remaining", data_source_number_to_retrieve,
        data_source_number_remaining > 0 ? data_source_number_remaining : 0);
  }
  return true;
}

bool DataVerifier::VerifyEqualityForRandomDataSet(int limit, int offset) {
  int data_source_number_remaining = limit;
  int data_source_number_to_retrieve =
      limit > number_of_entries_to_compare_each_turn_
          ? number_of_entries_to_compare_each_turn_
          : limit;

  CDCF_LOGGER_INFO("VerifyEqualityForRandomDataSet with limit {} & offset {}",
                   limit, offset);

  while (data_source_number_remaining > 0) {
    auto data_entries =
        data_source_a_->GetDataEntries(data_source_number_to_retrieve, offset);

    for (const auto& data_entry : data_entries) {
      if (!data_source_b_->FindIfDataEntryExists(data_entry)) {
        CDCF_LOGGER_ERROR(
            "cannot find corresponding data entry {} from second data source",
            data_entry);
        return false;
      }
    }

    data_source_number_remaining -= data_source_number_to_retrieve;
    offset += data_source_number_to_retrieve;
    CDCF_LOGGER_INFO(
        "{} entries compared with {} remaining", data_source_number_to_retrieve,
        data_source_number_remaining > 0 ? data_source_number_remaining : 0);
  }
  return true;
}
