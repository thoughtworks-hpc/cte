/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "./order_generator.h"

int GnenerateRandomNumber(int range_min, int range_max){
  return rand() % range_max + range_min;
}

void CreateDatabaseOrder(){
  std::string resp;
  int ret;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");
  ret = influxdb_cpp::create_db(resp, "orders", si);
  if (0 == ret) {
    std::cout << "creat db success, resp:" << resp << std::endl;
  } else {
    std::cout << "creat db failed ret:" << ret << std::endl;
  }
}

std::map<int, int> GenerateInitialPrice(int object_id_min, int object_id_max, int price_min,
                          int price_max) {
  srand((unsigned)time(nullptr));
  std::map<int, int> AllInitialPrice;

  std::string resp;
  int ret;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");

  for (int i = object_id_min; i < object_id_max + 1; i++){
    int price = GnenerateRandomNumber(price_min * 100, price_max * 100);
    AllInitialPrice[i] = price;

    ret = influxdb_cpp::builder()
        .meas("initial_price")
        .tag("key_symbol", std::to_string(i))
//        .field("symbol", i)
        .field("price", price)
        .timestamp(0)
        .post_http(si, &resp);

    if (0 == ret && "" == resp) {
      std::cout << "write db success" << std::endl;
    } else {
      std::cout << "write db failed, ret:" << ret << " resp:" << resp
                << std::endl;
    }
  }

  for (auto &item: AllInitialPrice){
    std::cout << item.first << " " << item.second <<std::endl;
  }


}

std::map<int, int> GetAllInitialPrice(){
  using json = nlohmann::json;

  std::map<int, int> result;
  std::string resp;
  int ret;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");

  ret = influxdb_cpp::query(resp, "select * from initial_price where ""key_symbol"" = '0'", si);
  if (0 == ret) {
    std::cout << "query db success, resp:" << resp << std::endl;
  } else {
    std::cout << "query db failed ret:" << ret << std::endl;
  }

  json j = json::parse(resp);
  std::cout << j["results"][0]["series"][0]["values"][0][2] << std::endl;
  return result;
}

int main(){
//  CreateDatabaseOrder();
//  GenerateInitialPrice(0,10,1,20);
  GetAllInitialPrice();
  return 0;
}