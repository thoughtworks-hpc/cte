/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/data_source_mock.h"

int DataSourceMock::GetDataEntryNumber() { return record_.size(); }

void DataSourceMock::GetDataEntries(int limit, int offset, std::string& data) {
  if (offset + limit > record_.size()) {
    limit = record_.size() - offset;
  }
  for (int i = offset; i < offset + limit; ++i) {
    record_.set(i);
  }
}

std::function<bool(const std::string& source, const std::string& target)>
DataSourceMock::GetCompareFunction() {
  return [](const std::string& a, const std::string& b) { return true; };
}

bool DataSourceMock::IfGotAllDataEntries() { return record_.all(); }
