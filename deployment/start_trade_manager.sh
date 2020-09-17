#!/usr/bin/env bash

sleep 5
/bin/trade_persistence_tool_main -t cte_trades -m $CTE_MATCH_ENGIN:51001 -d $DB_HOST:8086 -l debug -f /tmp/log/cte_trade_persistence_tool.log &
/bin/trade_persistence_tool_main -t akka_te_trades -m $AKKA_MATCH_ENGIN:50051 -d $DB_HOST:8086 -l debug -f /tmp/log/akka_te_trade_persistence_tool.log