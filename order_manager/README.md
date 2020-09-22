# Usage
> -a, --service_address: Specify order manager service address;

> -d, --database_address: Specify database address which is used by the order manager

> -m, --match_engine_main_address: Specify match engine main address, main address supports both order matching request and result subscription

> -r, --match_engine_request_addresses: Specify match engine request only addresses, request only address supports only order matching request

> -l, --log_level:  Log level, "info" or "debug", default: info

> -f, --log_file:  Log file location, default: /tmp/order_manager.log

> --db_user: InfluxDB username

> --db_password: InfluxDB password

> --db_name: Intended InfluxDB database name to use, default: order_mananger

> --db_measurement: Intended InfluxDB measurement name to use, default: order

> -h, --help: Print Usage
# Example

```shell script
./order_manager -a localhost:50051 -d localhost:8086 -m localhost:50052 -r localhost:50053,localhost:50054 --db_name order_manager
```
# Order Storage Location
Orders will be persisted to the specified influxdb in database "order_manager" with measurement of "order'.
# Database Schema
Orders will be persisted to the database in the following form:
```text
time                        amount concluded amount order_id price status              symbol_id trading_side user_id
----                        ------ ---------------- -------- ----- ------              --------- ------------ -------
2020-09-02T06:15:37.678617Z 10     10               1        7     concluded           1         buy          1
2020-09-02T06:15:44.145088Z 5      2                2        7     partially concluded 1         sell         2
```
time: the generated timestamp when receiving order

amount: the amount of symbol to buy or sell

concluded amount: the amount of the order that has been concluded through transaction

order_id: generated unique identification of the order

price: the price at which to buy or sell the symbol

status: status of the order

    submitted: the order has been submitted to the match engine, no result yet
    
    partially concluded: the order has been partially concluded
    
    concluded: the order has been fully concluded
    
symbol_id: the symbol id representing the symbol to buy or sell

trading_side: the trading side the order is at, buy or sell

user_id: user identification
