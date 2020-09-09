## Config File


This file defines  basic configurations for request generator, you can modify it to meet your needs.

**src / request_generator_config.json**
```
{
  "default_num_of_threads": 4,
  "default_num_of_requests": -1,
  "default_db_ip_address": "127.0.0.1",
  "default_db_port": "8086",
  "default_db_name": "orders",
  "database_user_name": "",
  "database_password": "",
  "grpc_server": [
    {
      "ip": "127.0.0.1",
      "port": "50051"
    },
    {
      "ip": "127.0.0.1",
      "port": "50052"
    },
    {
      "ip": "127.0.0.1",
      "port": "50053"
    }
  ]
}
```
>  note: "default_num_of_requests : -1" means  generator will send all orders in database. 


## Usage Example

```
./request_generator_main -t 4 -n 10 -f [USER_DEFINE_CONFIG_FILE_PATH]
```

> -t, can overwrite the number of threads in config file

> -n can overwrite the number of requests to be sent in. config file

> -f define the config file path, default is "./request_generator_config.json"
