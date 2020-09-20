#!/usr/bin/env bash
/bin/order_manager -a 0.0.0.0:$ORDER_MANAGER_PORT -d $DB_HOST:8086 -m $MATCH_ENGIN:$MATCH_ENGIN_PORT -t $DEBUG