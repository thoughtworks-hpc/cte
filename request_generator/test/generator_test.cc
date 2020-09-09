/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <gmock/gmock.h>
#include <grpcpp/create_channel.h>

#include "../../common/include/influxdb.hpp"
#include "./config_manager_test.h"
#include "./server.h"
#include "include/config.h"

using testing::Eq;

void Clean() {
  Generator::requests_count_ = 0;
  Generator::count_each_server_.clear();
}

class MockDatabase : public DatabaseQueryInterface {
 public:
  std::string GetOrders() override {
    return "{\"results\":[{\"statement_id\":0,\"series\":[{\"name\":\"orders\","
           "\"columns\":[\"time\",\"amount\",\"price\",\"symbol\",\"trading_"
           "side\",\"user_id\"],\"values\":[[1,1,1,1,1,1],[2,2,2,2,2,2],[3,3,3,"
           "3,3,3],[4,4,4,4,4,4]]}]}]}";
  }
};

TEST(ConfigTest, check_config_initialization) {
  char** config_file_path = new char* [2] { INI_FILE_PARAMETER };
  char* file_path = config_file_path[0];
  char file[] = "-f";
  char* argv[3]{file_path, file, file_path};

  request_generator::Config config(3, argv);
  EXPECT_THAT(config.num_of_requests_, Eq(1));
  EXPECT_THAT(config.num_of_threads_, Eq(1));
  EXPECT_THAT(config.database_ip_, Eq("127.0.0.1"));
  EXPECT_THAT(config.database_port_, Eq("8086"));
  EXPECT_THAT(config.database_user_name_, Eq(""));
  EXPECT_THAT(config.database_password_, Eq(""));
  EXPECT_THAT(config.database_name_, Eq("orders"));
  EXPECT_THAT(config.grcp_ip_address_.size(), Eq(1));
  EXPECT_THAT(config.grcp_ip_address_[0].ip_, Eq("127.0.0.1"));
  EXPECT_THAT(config.grcp_ip_address_[0].port_, Eq("50051"));
}

TEST(SingleThreadTest,
     should_count_one_message_for_each_server_when_send_three_messages) {
  std::string port = "50051";
  OrderManagerImpl grpc_server_1(port);
  grpc_server_1.Run();

  port = "50052";
  OrderManagerImpl grpc_server_2(port);
  grpc_server_2.Run();

  port = "50053";
  OrderManagerImpl grpc_server_3(port);
  grpc_server_3.Run();

  auto mockDatabase = new MockDatabase();
  std::vector<ip_address> grcp_ip_address{ip_address("127.0.0.1", "50051"),
                                          ip_address("127.0.0.1", "50052"),
                                          ip_address("127.0.0.1", "50053")};

  Generator generator(1, 3, grcp_ip_address, mockDatabase);
  generator.Start();

  const std::vector<int> count_each_server = Generator::count_each_server_;

  EXPECT_THAT(count_each_server[0], Eq(1));
  EXPECT_THAT(count_each_server[1], Eq(1));
  EXPECT_THAT(count_each_server[2], Eq(1));
  EXPECT_THAT(Generator::requests_count_, Eq(3));
  Clean();
}

TEST(SingleThreadTest,
     should_count_one_message_when_send_single_message_to_single_server) {
  std::string port = "50055";
  OrderManagerImpl grpc_server(port);
  grpc_server.Run();

  auto mockDatabase = new MockDatabase();
  std::vector<ip_address> grcp_ip_address{ip_address("127.0.0.1", "50055")};
  Generator generator(1, 1, grcp_ip_address, mockDatabase);
  generator.Start();

  const std::vector<int> count_each_server = Generator::count_each_server_;
  for (int i = 0; i < count_each_server.size(); i++) {
    std::cout << "Send " << count_each_server[i] << " requests to "
              << grcp_ip_address[i].ip_ << ":" << grcp_ip_address[i].port_
              << std::endl;
  }
  EXPECT_THAT(count_each_server[0], Eq(1));
  EXPECT_THAT(Generator::requests_count_, Eq(1));
  Clean();
}

TEST(MultipleThreadsTest,
     should_count_three_messages_for_one_server_when_send_three_message) {
  std::string port = "50051";
  OrderManagerImpl grpc_server_1(port);
  grpc_server_1.Run();

  auto mockDatabase = new MockDatabase();
  std::vector<ip_address> grcp_ip_address{ip_address("127.0.0.1", "50051")};
  std::cout << "size:" << grcp_ip_address.size() << std::endl;

  Generator generator(3, 3, grcp_ip_address, mockDatabase);
  generator.Start();

  const std::vector<int> count_each_server = Generator::count_each_server_;

  EXPECT_THAT(count_each_server[0], Eq(3));
  EXPECT_THAT(Generator::requests_count_, Eq(3));
  Clean();
}

TEST(MultipleThreadsTest, two_threads_send_two_messages_to_two_server) {
  std::string port = "50051";
  OrderManagerImpl grpc_server_1(port);
  grpc_server_1.Run();

  port = "50052";
  OrderManagerImpl grpc_server_2(port);
  grpc_server_2.Run();

  auto mockDatabase = new MockDatabase();
  std::vector<ip_address> grcp_ip_address{ip_address("127.0.0.1", "50051"),
                                          ip_address("127.0.0.1", "50052")};
  std::cout << "size:" << grcp_ip_address.size() << std::endl;

  Generator generator(2, 4, grcp_ip_address, mockDatabase);
  generator.Start();

  const std::vector<int> count_each_server = Generator::count_each_server_;

  EXPECT_THAT(count_each_server[0], Eq(2));
  EXPECT_THAT(count_each_server[1], Eq(2));
  EXPECT_THAT(Generator::requests_count_, Eq(4));
  Clean();
}
