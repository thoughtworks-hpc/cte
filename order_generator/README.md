# order_generator
This data generator contains two part: **create initial prices** and **create orders**. Both of them are executable files.

## Create initial prices

In this part, a database named "orders" will be created and initial prices can be created by **create_initial_price.cc** with a default config file (**create_initial_prices_config.json**) in the same src directory. Or you can choose your own json file by adding a parameter which should be your json file path as the program arguments.

The config file should in this form, with four arguments:

```json
{
  "symbol_id_min": 3,
  "symbol_id_max": 10,
  "price_min": 1,
  "price_max": 100
}
```

Initial prices will be created in a json file in your current working directory named **initial_prices.json**. In this file, initial prices are shown like`{"symbol_id": initial price}`.

> If you want to use the default config file, please make sure your current working dierectory is the same as the excutable file path.

## Create orders

In this part, orders can be created by **create_orders.cc** with two default files(**initial_prices.json** and **create_orders_config.json**) in the same src directory. Or you can choose your own json files by adding two parameters which should be your json file pathes as the program arguments, in which the first one is like **initial_prices.json** and the second one is like **create_orders_config.json**.

The forms of  **initial_prices.json** and **create_orders_config.json** are shown:

initial_prices.json:

```json
{"1":5679,"10":8759,"2":3175,"3":5325,"4":5199,"5":6861,"6":5637,"7":3839,"8":9626,"9":665}
```

create_orders_config.json:

```json
{
  "order_amount": 10000,
    "user_id_min": 1,
    "user_id_max": 100,
    "trading_amount_min": 100,
    "trading_amount_max": 1000,
    "database_host": "127.0.0.1",
    "database_port": 8086,
    "database_name": "orders",
    "database_user": "",
    "database_password": ""
}
```

Orders will be created in the influxdb database, whose database name and measurement are "orders".

> If you want to use the default config file, please make sure your current working dierectory is the same as the excutable file path.