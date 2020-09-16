/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_SOURCE_MOCK_H_
#define DATA_VERIFIER_INCLUDE_DATA_SOURCE_MOCK_H_

#include <bitset>
#include <mutex>
#include <string>
#include <vector>

#include "./data_source.h"

class DataSourceMock : public DataSource {
 public:
  int GetDataEntryNumber() override;
//  void GetDataEntries(int limit, int offset, std::string& data) override;
  std::vector<std::string> GetDataEntries(int limit, int offset) override;
  bool CompareDataEntry(const std::string& source,
                        const std::string& target) override;
  bool FindIfDataEntryExists(const std::string& entry) override;
//  std::function<bool(const std::string& source, const std::string& target)>
//  GetCompareFunction() override;
  bool IfGotAllDataEntries();

 private:
  std::bitset<27000> record_;
  mutable std::mutex record_mutex_;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_SOURCE_MOCK_H_
