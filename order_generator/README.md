# order_generator
This data generator contains two part: **create initial prices** and **create orders**. Both of them are executable files.

## Create initial prices

In this part, initial prices can be created by **create_initial_price.cc** with a default config file (**create_initial_prices_config.json**) in the same src directory. Or you can choose your own json file by adding a parameter which should be your json file path as the program arguments.

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

## Create orders

In this part, orders can be created by **create_orders.cc** with two default config files(**create_initial_prices_config.json** and **create_orders_config.json**) in the same src directory. Or you can choose your own json files by adding two parameters which should be your json file pathes as the program arguments, in which the first one is like **create_initial_prices_config.json** and the second one is like **create_orders_config.json**.

The form of  **create_initial_prices_config.json** is the same as before, while the form of **create_orders_config.json** should like the following one, with seven arguments:

```json
{
  "order_amount": 10,
  "user_id_min": 1,
  "user_id_max": 100,
  "symbol_id_min": 3,
  "symbol_id_max": 10,
  "trading_amount_min": 100,
  "trading_amount_max": 1000
}
```

Orders will be created in the influxdb database, whose database name and measurement are "orders".


