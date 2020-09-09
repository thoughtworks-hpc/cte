/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_DATA_SOURCE_MOCK_H
#define CTE_DATA_SOURCE_MOCK_H

#include <bitset>
#include <mutex>
#include <string>
#include <vector>

#include "./data_source.h"

class DataSourceMock : public DataSource {
 public:
  int GetDataEntryNumber() override;
  void GetDataEntries(int limit, int offset, std::string& data) override;
  std::function<bool(const std::string& source, const std::string& target)>
  GetCompareFunction() override;
  bool IfGotAllDataEntries();
 private:
  std::bitset<270> record_;
  mutable std::mutex record_mutex_;
};

#endif  // CTE_DATA_SOURCE_MOCK_H
