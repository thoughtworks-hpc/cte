#!/usr/bin/env bash

sleep 5
/bin/trade_persistence_tool_main -t cte_trades -m $CTE_MATCH_ENGIN:51001 -d $DB_HOST:8086 -l debug -f /tmp/cte_trade_persistence_tool.log
#/bin/trade_persistence_tool_main -t akka_te_trades -m akkagateway:50051 -d 127.0.0.1:8086 -l debug -f /tmp/akka_te_trade_persistence_tool.log