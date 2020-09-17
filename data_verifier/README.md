# Usage:
Directly run data_verifier, configuration must be set via json file.
# Configuration:
"server": must specify 2 influxdb server as 2 data sources to compare

"level_log": log level which typically has value "info" or "debug".

"ordered_data_sources": whether the data sources has ordered data sets.
Comparison between ordered data sources will be much faster.

"compare_entire_data_source_in_one_turn", "number_of_entries_to_compare_each_turn": 
whether to compare every data entries from 2 data sources in one turn 
or in several turns and in each turn compare [number_of_entries_to_compare_each_turn] of entries. 
# Example:
> ./data_verifier

With the following json file named "data_verifier_config.json" in the same directory.
```json
{
    "server": [
      {
        "host": "127.0.0.1",
        "port": 55555,
        "database": "trade_manager",
        "measurement": "cte_trades",
        "user": "",
        "password": ""
      },
      {
        "host": "127.0.0.1",
        "port": 55555,
        "database": "trade_manager",
        "measurement": "cte_trades",
        "user": "",
        "password": ""
      }
    ],
    "level_log": "info",
    "ordered_data_sources": true,
    "compare_entire_data_source_in_one_turn": true,
    "number_of_entries_to_compare_each_turn": 10000
}
```