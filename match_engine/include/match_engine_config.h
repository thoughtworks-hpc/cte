/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef MATCH_ENGINE_INCLUDE_MATCH_ENGINE_CONFIG_H_
#define MATCH_ENGINE_INCLUDE_MATCH_ENGINE_CONFIG_H_

#include <cdcf/actor_system.h>

#include <string>
#include <vector>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"

// template <class Inspector>
// typename Inspector::result_type inspect(Inspector& f, const
// ::match_engine_proto::Order& x) {
//  return f(caf::meta::type_name("match_engine_proto"), x.order_id(), );
//
namespace match_engine {

const int64_t k_send_match_result_timeout = 2;
using MatchResultWriter = ::grpc::ServerWriter< ::match_engine_proto::Trade>*;

struct MatchResultWriterKeeper {
  MatchResultWriter writer;
  std::promise<int>* writer_promise;
};

using MatchResultWriteKeepers = std::vector<MatchResultWriterKeeper>;

using GetAtom = caf::atom_constant<caf::atom("get")>;

struct SymbolActorInfo {
  int32_t symbol_id;
  caf::actor symbol_actor;
  SymbolActorInfo() {}
  SymbolActorInfo(int32_t symbolId, const caf::actor& symbolActor)
      : symbol_id(symbolId), symbol_actor(symbolActor) {}
};

enum TRADING_SITE_E { TRADING_SITE_BUY, TRADING_SITE_SELL };

struct RawOrder {
  int64_t order_id;
  int32_t symbol_id;
  int32_t user_id;
  TRADING_SITE_E trading_side;
  int32_t amount;
  int32_t price;
  int64_t submit_time;  // 订单提交时间
};

struct MatchedTrade {
  int64_t maker_id;             // maker订单id
  int64_t taker_id;             // taker订单id， taker吃掉maker
  TRADING_SITE_E trading_side;  // 吃单的买卖方向
  int32_t amount;
  int32_t price;
  int32_t seller_user_id;
  int32_t buyer_user_id;
  int32_t symbol_id;
  int64_t submit_time;
  int64_t deal_time; // 成交时间
  MatchedTrade() {}
  MatchedTrade(int64_t makerId, int64_t takerId, TRADING_SITE_E tradingSide,
               int32_t amount, int32_t price, int32_t sellerUserId,
               int32_t buyerUserId, int32_t symbolId, int64_t submitTime, int64_t dealTime)
      : maker_id(makerId),
        taker_id(takerId),
        trading_side(tradingSide),
        amount(amount),
        price(price),
        seller_user_id(sellerUserId),
        buyer_user_id(buyerUserId),
        symbol_id(symbolId),
        submit_time(submitTime),
        deal_time(dealTime) {}
};

using TradeList = std::vector<MatchedTrade>;

struct TradeListMsg {
  TradeList data;
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, const RawOrder& x) {
  return f(caf::meta::type_name("RawOrder"), x.order_id, x.symbol_id, x.user_id,
           x.trading_side, x.amount, x.price, x.submit_time);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f,
                                        const SymbolActorInfo& x) {
  return f(caf::meta::type_name("SymbolActorInfo"), x.symbol_id,
           x.symbol_actor);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, const MatchedTrade& x) {
  return f(caf::meta::type_name("MatchedTrade"), x.maker_id, x.taker_id,
           x.trading_side, x.amount, x.price, x.seller_user_id, x.buyer_user_id,
           x.symbol_id, x.submit_time);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, const TradeListMsg& x) {
  return f(caf::meta::type_name("TradeListMsg"), x.data);
}

class SenderMatchInterface {
 public:
  virtual void SendMatchResult(const TradeList& trade_list) = 0;
};

static const char kResultHostRoleName[] = "merge_result_host";

class Config : public cdcf::actor_system::Config {
 public:
  uint16_t grpc_server_port = 51001;
  uint16_t match_router_port = 51020;
  std::string symbol_id_list;
  uint16_t merge_result_port = 51021;
  Config() {
    add_message_type<RawOrder>("RawOrder");
    add_message_type<SymbolActorInfo>("SymbolActorInfo");
    add_message_type<MatchedTrade>("MatchedTrade");
    add_message_type<TradeListMsg>("TradeListMsg");
    opt_group{custom_options_, "global"}
        .add(grpc_server_port, "grpc_server_port", "GRPC server port")
        .add(symbol_id_list, "symbol_id_list", "symbol list")
        .add(merge_result_port, "merge_result_port",
             "Match result port, if set 0, this node will not merge result")
        .add(match_router_port, "match_router_port",
             "match router publish port");
  }
};

}  // namespace match_engine

#endif  // MATCH_ENGINE_INCLUDE_MATCH_ENGINE_CONFIG_H_
