# Mini Redis

A lightweight implementation of a Redis server written in C++ 17. This project was built primarily to deepen my understanding of how Redis works and to practice various programming concepts.

## Features
Supports the following list of commands:
- PING
- ECHO
- SET/GET
- DEL
- EXISTS
- EXPIRE
- TTL
- INCR/DECR
- INCRBY/DECRBY
- LPUSH/RPUSH
- LPOP/RPOP
- LINDEX
- LLEN
- SAVE


## Building locally
Ensure that you have a C++ compiler installed.\
Run the build script: `./build.sh`, you may need to grant execute permissions by running: `chmod +x ./build.sh` \
Start the server: `./main`\
You can use the [official CLI tool](https://redis.io/docs/latest/develop/tools/cli/) to send requests to the server

## Tests

Tests are integrated using Catch2. To run tests, build the project as described above, and run `ctest --output-on-failure`
