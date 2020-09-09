/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_INCLUDE_UUID_H_
#define TRADE_PERSISTENCE_TOOL_INCLUDE_UUID_H_

#include <random>
#include <sstream>
#include <string>

namespace uuid {
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

std::string generate_uuid_v4();
}  // namespace uuid

#endif  // TRADE_PERSISTENCE_TOOL_INCLUDE_UUID_H_
