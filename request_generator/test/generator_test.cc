/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <gmock/gmock.h>
#include <grpcpp/create_channel.h>

#include "./server.h"
#include "src/config.h"

using testing::Eq;

TEST(ConfigTest, check_config_initialization) {
  request_generator::Config config("request_generator_config_test.json");
  EXPECT_THAT(config.default_num_of_requests_, Eq(1));
  EXPECT_THAT(config.default_num_of_threads_, Eq(1));
  EXPECT_THAT(config.default_db_host_address_, Eq("127.0.0.1"));
  EXPECT_THAT(config.default_db_port_, Eq("8086"));
  EXPECT_THAT(config.grcp_ip_address_.size(), Eq(1));
  EXPECT_THAT(config.grcp_ip_address_[0].ip_, Eq("127.0.0.1"));
  EXPECT_THAT(config.grcp_ip_address_[0].port_, Eq("8086"));
}

// TEST(GRPCTest, check_grpc_connection) {
//  std::string port = "50055";
//  std::cout << "This is grpc test" << std::endl;
//  OrderManagerImpl order_manager_impl(port);
//  order_manager_impl.Run();
//
//  std::vector<ip_address> grcp_ip_address{ip_address("127.0.0.1", "50055")};
//  Generator generator(1, 1, grcp_ip_address, "127.0.0.1", "8086");
//  generator.Start();
//  EXPECT_THAT(Generator::GetRequestsCount(), Eq(1));
//}
