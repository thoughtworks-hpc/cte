/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/data_source_mock.h"

int DataSourceMock::GetDataEntryNumber() { return record_.size(); }

std::vector<std::string> DataSourceMock::GetDataEntries(int limit, int offset) {
  if (offset + limit > record_.size()) {
    limit = record_.size() - offset;
  }
  record_mutex_.lock();
  for (int i = offset; i < offset + limit; ++i) {
    record_.set(i);
  }
  record_mutex_.unlock();

  return std::vector<std::string>();
}

bool DataSourceMock::CompareDataEntry(const std::string& source,
                                      const std::string& target) {
  return true;
}

bool DataSourceMock::FindIfDataEntryExists(const std::string& entry) {
  return true;
}

bool DataSourceMock::IfGotAllDataEntries() { return record_.all(); }
