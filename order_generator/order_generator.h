/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_GENERATOR_H
#define CTE_ORDER_GENERATOR_H

#include <iostream>

class Order{
 public:
 private:
  int user_id_;
  int object_id_;
  int price_;
  int initial_price_;
  int amount_;
  std::string side_;
};

#endif  // CTE_ORDER_GENERATOR_H
