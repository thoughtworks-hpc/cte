#!/usr/bin/env bash
/usr/bin/influxd &
sleep 10
/bin/create_initial_prices
/bin/create_orders
