#!/usr/bin/env bash
echo "[IMPORTANT] -a 0.0.0.0:$ORDER_MANAGER_PORT -d $DB_HOST:8086 -m $MATCH_ENGIN:$MATCH_ENGIN_PORT -t $DEBUG -f /tmp/log/order_manager.log -l debug"
/bin/order_manager -a 0.0.0.0:$ORDER_MANAGER_PORT -d $DB_HOST:8086 -db_name akka_order_manager -m $MATCH_ENGIN:$MATCH_ENGIN_PORT -t $DEBUG -f /tmp/log/order_manager.log -l debug