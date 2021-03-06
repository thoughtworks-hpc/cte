/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_SOURCE_MOCK_H_
#define DATA_VERIFIER_INCLUDE_DATA_SOURCE_MOCK_H_

#include <bitset>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "./data_source.h"

class DataSourceMock : public DataSource {
 public:
  int GetDataEntryNumber() override;
  std::vector<std::string> GetDataEntries(int limit, int offset) override;
  std::unordered_map<std::string, std::vector<std::string>>
  GetDataEntriesBySymbol(int limit, int offset) override;
  bool CompareDataEntry(const std::string& source,
                        const std::string& target) override;
  bool FindIfDataEntryExists(const std::string& entry) override;
  std::string GetDataSourceName() override { return std::string(); }
  std::string GetDataEntryDebugString(const std::string& entry) override {
    return std::string();
  }

  bool IfGotAllDataEntries();

 private:
  std::bitset<27000> record_;
  mutable std::mutex record_mutex_;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_SOURCE_MOCK_H_
