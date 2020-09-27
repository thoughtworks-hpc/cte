/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_SOURCE_H_
#define DATA_VERIFIER_INCLUDE_DATA_SOURCE_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class DataSource {
 public:
  virtual int GetDataEntryNumber() = 0;
  virtual std::vector<std::string> GetDataEntries(int limit, int offset) = 0;
  virtual std::unordered_map<std::string, std::vector<std::string>>
  GetDataEntriesBySymbol(int limit, int offset) = 0;
  virtual bool CompareDataEntry(const std::string& source,
                                const std::string& target) = 0;
  virtual bool FindIfDataEntryExists(const std::string& entry) = 0;
  virtual std::string GetDataSourceName() = 0;
  virtual std::string GetDataEntryDebugString(const std::string& entry) = 0;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_SOURCE_H_
