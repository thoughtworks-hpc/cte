# How to backup order data
1. enter the request generator docker container where order data resides:
    > docker exec -it docker_request_generator_1 /bin/bash
2. backup the database, must use /tmp directory since it's mapped to host machine:
    > influxd backup -portable -database [your_database_name] -host 127.0.0.1:8088 /tmp/[name-to-backup]
3. later restore the database:
    > influxd restore -portable /tmp/[name-to-backup]
# Example
1. backup:
    > influxd backup -portable -database orders -host 127.0.0.1:8088 /tmp/orders
2. restore:
    > influxd restore -portable /tmp/orders