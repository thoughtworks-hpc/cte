/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */
#include "../include/data_verifier.h"

#include <cdcf/logger.h>

#include <iostream>
#include <thread>

bool DataVerifier::VerifyEquality() {
  std::string data_a;
  std::string data_b;

  int data_source_a_number = data_source_a_->GetDataEntryNumber();
  int data_source_b_number = data_source_b_->GetDataEntryNumber();

  if (data_source_a_number != data_source_b_number) {
    CDCF_LOGGER_INFO("trade data entry number inconsistent between {} and {}",
                     data_source_a_number, data_source_b_number);
    return false;
  }

  int data_source_number_remaining = data_source_a_number;
  int data_source_number_to_retrieve = 100;
  int offset = 0;

  while (data_source_number_remaining > 0) {
    data_source_a_->GetDataEntries(data_source_number_to_retrieve, offset,
                                   data_a);
    data_source_b_->GetDataEntries(data_source_number_to_retrieve, offset,
                                   data_b);

    if (!data_source_a_->GetCompareFunction()(data_a, data_b)) {
      return false;
    }

    data_source_number_remaining -= data_source_number_to_retrieve;
    offset += data_source_number_to_retrieve;
  }

  return true;
}
