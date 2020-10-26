/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/match_actor.h"

#include <cdcf/logger.h>

#include <chrono>

namespace match_engine {

void MakeTrade(RawOrder& maker_order, RawOrder& taker_order,
               std::vector<MatchedTrade>& trade_list) {
  int32_t trade_amount = 0;
  int64_t submit_time;
  if (maker_order.amount < taker_order.amount) {
    trade_amount = maker_order.amount;
    maker_order.amount = 0;
    taker_order.amount -= trade_amount;
    submit_time = maker_order.submit_time;
  } else {
    trade_amount = taker_order.amount;
    taker_order.amount = 0;
    maker_order.amount -= trade_amount;
    submit_time = taker_order.submit_time;
  }

  int32_t seller_user_id = 0;
  int32_t buyer_user_id = 0;

  if (TRADING_SITE_BUY == taker_order.trading_side) {
    seller_user_id = maker_order.user_id;
    buyer_user_id = taker_order.user_id;
  } else {
    seller_user_id = taker_order.user_id;
    buyer_user_id = maker_order.user_id;
  }

  int64_t deal_time =
      std::chrono::system_clock::now().time_since_epoch().count();

  trade_list.emplace_back(maker_order.order_id, taker_order.order_id,
                          taker_order.trading_side, trade_amount,
                          maker_order.price, seller_user_id, buyer_user_id,
                          taker_order.symbol_id, submit_time, deal_time);
}

bool MatchOrderFromList(RawOrderList& order_list, RawOrder& order,
                        std::vector<MatchedTrade>& trade_list) {
  for (auto maker_it = order_list.begin(); maker_it != order_list.end();) {
    if (TRADING_SITE_SELL == order.trading_side) {
      if (order.price > maker_it->price) {
        return false;
      }
    } else {
      if (order.price < maker_it->price) {
        return false;
      }
    }

    MakeTrade(*maker_it, order, trade_list);
    if (0 == maker_it->amount) {
      maker_it = order_list.erase(maker_it);
    } else {
      ++maker_it;
    }

    if (0 == order.amount) {
      return false;
    }
  }

  return true;
}

std::vector<MatchedTrade> MatchOrderFromTable(OrderTable& order_table,
                                              RawOrder& order) {
  std::vector<MatchedTrade> trade_list;
  for (auto order_list_it = order_table.begin();
       order_list_it != order_table.end();) {
    bool can_match = MatchOrderFromList(*order_list_it, order, trade_list);

    if (order_list_it->empty()) {
      order_list_it = order_table.erase(order_list_it);
    } else {
      ++order_list_it;
    }

    if (!can_match) {
      break;
    }
  }

  return trade_list;
}

void AddToOrderTable(OrderTable& order_table, RawOrder& order) {
  for (auto order_list_it = order_table.begin();
       order_list_it != order_table.end();) {
    auto& order_list = *order_list_it;

    if (order.price == order_list.begin()->price) {
      order_list.emplace_back(order);
      return;
    }

    bool need_create;

    if (TRADING_SITE_BUY == order.trading_side) {
      need_create = order.price > order_list.begin()->price;
    } else {
      need_create = order.price < order_list.begin()->price;
    }

    if (need_create) {
      order_table.insert(order_list_it, RawOrderList{order});
      return;
    }

    ++order_list_it;
  }

  order_table.emplace_back(RawOrderList{order});
}

void SendMatchResult(caf::stateful_actor<OrderBook>* sender,
                     const TradeList& match_result) {
  int result_count = match_result.size();
  if (0 == result_count) {
    return;
  }

  if (nullptr != sender->state.match_result_actor) {
    for (auto& one_match : match_result) {
      CDCF_LOGGER_DEBUG(
          "Match actor send match result, symbol id:{}, price: {}, amount:{}, "
          "taker id:{}, maker id:{}, trade side:{}, seller id:{}, buyer id:{}, "
          "match time:{}, submit time: {}",
          one_match.symbol_id, one_match.price, one_match.amount,
          one_match.taker_id, one_match.maker_id, one_match.trading_side,
          one_match.seller_user_id, one_match.buyer_user_id,
          one_match.deal_time, one_match.submit_time);
    }

    sender
        ->request(sender->state.match_result_actor,
                  // std::chrono::seconds(k_send_match_result_timeout),
                  caf::infinite, TradeListMsg{match_result})
        .then(
            [result_count](int) {
              CDCF_LOGGER_DEBUG(
                  "Send match result success, information count:{}",
                  result_count);
            },
            [result_count](caf::error& err) {
              CDCF_LOGGER_ERROR(
                  "Send match result failed, information count:{}",
                  result_count);
            });
  } else {
    CDCF_LOGGER_WARN(
        "Match sender is null, match result will not send, information "
        "count:{}",
        result_count);
  }
}

caf::behavior MatchActor(caf::stateful_actor<OrderBook>* self) {
  return {
      [=](RawOrder& order) {
        CDCF_LOGGER_DEBUG(
            "match actor Receive new order, id:{}, price:{}, amount:{}, trade "
            "site:{}, "
            "submit time: {}, "
            "symbol id:{}, user id:{}",
            order.order_id, order.price, order.amount, order.trading_side,
            order.submit_time, order.symbol_id, order.user_id);

        if (TRADING_SITE_BUY == order.trading_side) {
          auto trade_list = MatchOrderFromTable(self->state.sell_list, order);

          if (0 != order.amount) {
            AddToOrderTable(self->state.buy_list, order);
          }

          SendMatchResult(self, trade_list);
        } else {
          auto trade_list = MatchOrderFromTable(self->state.buy_list, order);

          if (0 != order.amount) {
            AddToOrderTable(self->state.sell_list, order);
          }

          SendMatchResult(self, trade_list);
        }
      },
      [=](caf::actor& result_receiver) {
        CDCF_LOGGER_DEBUG("set result_receiver success, actor id:{}",
                          self->id());
        self->state.match_result_actor = result_receiver;
      }};
}

}  // namespace match_engine
