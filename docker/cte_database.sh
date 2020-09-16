#!/usr/bin/env bash
/usr/bin/influxd &
sleep 10
cd bin
/bin/create_initial_prices
/bin/create_orders

sleep infinity