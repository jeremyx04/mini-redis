#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include "../src/linkedlist.h"
#include "../src/redisengine.h"
#include "../src/resp.h"
#include <thread>

TEST_CASE("Math works!") {
    REQUIRE(2+2 == 4); 
}

TEST_CASE("Serialize Simple String", "[RESP]") {
    SimpleString simple_str("Hello World!");
    REQUIRE(simple_str.serialize() == "+Hello World!\r\n");
}

TEST_CASE("Serialize Error", "[RESP]") {
    Error error("An error occurred...");
    REQUIRE(error.serialize() == "-An error occurred...\r\n");
}

TEST_CASE("Serialize Bulk String", "[RESP]") {
    BulkString bulk_str("This is a bulk string!");
    REQUIRE(bulk_str.serialize() == "$22\r\nThis is a bulk string!\r\n");
}

TEST_CASE("Serialize Integer", "[RESP]") {
    Integer x(22);
    REQUIRE(x.serialize() == ":22\r\n");
}

TEST_CASE("Serialize Array", "[RESP]") {
    std::vector<std::unique_ptr<RType>> nested;
    nested.push_back(std::make_unique<Integer>(1));
    nested.push_back(std::make_unique<Integer>(2));
    std::unique_ptr<Array> nested_arr = std::make_unique<Array>(nested);
    std::vector<std::unique_ptr<RType>> vec;  
    vec.push_back(std::make_unique<SimpleString>("Element 0"));
    vec.push_back(std::move(nested_arr));
    Array arr(vec);
    REQUIRE(arr.serialize() == "*2\r\n+Element 0\r\n*2\r\n:1\r\n:2\r\n");
}

TEST_CASE("Deserialize Simple String", "[RESP]") {
    std::unique_ptr<SimpleString> expect = std::make_unique<SimpleString>("Hello World!");
    std::unique_ptr<RType> deserialized = RType::deserialize("+Hello World!\r\n");
    REQUIRE(deserialized->serialize() == expect->serialize());
}

TEST_CASE("Deserialize Error", "[RESP]") {
    std::unique_ptr<Error> expect = std::make_unique<Error>("An error occurred...");
    std::unique_ptr<RType> deserialized = RType::deserialize("-An error occurred...\r\n");
    REQUIRE(deserialized->serialize() == expect->serialize());
}

TEST_CASE("Deserialize Bulk String", "[RESP]") {
    std::unique_ptr<BulkString> expect = std::make_unique<BulkString>("This is a bulk string!");
    std::unique_ptr<RType> deserialized = RType::deserialize("$22\r\nThis is a bulk string!\r\n");
    REQUIRE(deserialized->serialize() == expect->serialize());
}

TEST_CASE("Deserialize Integer", "[RESP]") {
    std::unique_ptr<Integer> expect = std::make_unique<Integer>(22);
    std::unique_ptr<RType> deserialized = RType::deserialize(":22\r\n");
    REQUIRE(deserialized->serialize() == expect->serialize());
}

TEST_CASE("Deserialize Array", "[RESP]") {
    std::unique_ptr<RType> basic_deserialized = RType::deserialize("*1\r\n+hi\r\n");
    std::vector<std::unique_ptr<RType>> basic_lst;  
    basic_lst.push_back(std::make_unique<SimpleString>("hi"));
    std::unique_ptr<Array> basic_expect = std::make_unique<Array>(basic_lst);
    REQUIRE(basic_deserialized->serialize() == basic_expect->serialize());

    std::unique_ptr<RType> nested_deserialized = RType::deserialize("*2\r\n+Element 0\r\n*2\r\n*1\r\n:1\r\n:2\r\n");
    std::vector<std::unique_ptr<RType>> inner_nest;
    inner_nest.push_back(std::make_unique<Integer>(1));
    
    std::vector<std::unique_ptr<RType>> outer_nest;
    outer_nest.push_back(std::make_unique<Array>(inner_nest));
    outer_nest.push_back(std::make_unique<Integer>(2));

    std::vector<std::unique_ptr<RType>> nested_lst;  
    nested_lst.push_back(std::make_unique<SimpleString>("Element 0"));
    nested_lst.push_back(std::move(std::make_unique<Array>(outer_nest)));
    std::unique_ptr<Array> nested_expect = std::make_unique<Array>(nested_lst);
    
    REQUIRE(nested_deserialized->serialize() == nested_expect->serialize());
}

TEST_CASE("PING command returns PONG", "[redis]") {
    RedisEngine redis = RedisEngine();
    std::string request = "*1$4\r\nPING\r\n";
    auto response = redis.handle_request(request);
    REQUIRE(dynamic_cast<SimpleString*>(response.get())->get_str() == "PONG");
}

TEST_CASE("SET/GET commands work correctly", "[redis]") {
    RedisEngine redis = RedisEngine();

    std::string set_request = "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n";
    auto set_response = redis.handle_request(set_request);
    REQUIRE(dynamic_cast<SimpleString*>(set_response.get())->get_str() == "OK");

    std::string get_request = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";
    auto get_response = redis.handle_request(get_request);
    REQUIRE(dynamic_cast<SimpleString*>(get_response.get())->get_str() == "value");
}

TEST_CASE("Multithreaded INCR works", "[redis]") {
    RedisEngine redis = RedisEngine();

    const int num_threads = 10; 
    const int num_operations = 100; 
    const std::string key = "key";

    std::string init_request = "*3\r\n$3\r\nSET\r\n$" +
                std::to_string(key.size()) + "\r\n" + key +
                "\r\n$1" + "\r\n" + "0" + "\r\n";
    auto init_response = redis.handle_request(init_request);
    REQUIRE(dynamic_cast<SimpleString*>(init_response.get())->get_str() == "OK");

    auto thread_function = [&](int thread_id) {
        for (int i = 0; i < num_operations; ++i) {
            std::string incr_request = "*2\r\n$4\r\nINCR\r\n$" + std::to_string(key.size()) + "\r\n" + key + "\r\n";
            auto incr_response = redis.handle_request(incr_request);
            REQUIRE(dynamic_cast<Integer*>(incr_response.get())->get_val() >= 0);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(thread_function, i);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::string get_request = "*3\r\n$3\r\nGET\r\n$" +
                std::to_string(key.size()) + "\r\n" + key + "\r\n";
    auto get_response = redis.handle_request(get_request);
    REQUIRE(dynamic_cast<SimpleString*>(get_response.get())->get_str() == "1000");
}

TEST_CASE("LinkedList works", "[linkedlist]") {
    LinkedList lst = LinkedList();

    lst.push_back("val_1");
    REQUIRE(lst.get_size() == 1);
    REQUIRE(lst.pop_back() == "val_1");

    lst.push_front("val_2");
    REQUIRE(lst.get_size() == 1);
    REQUIRE(lst.pop_front() == "val_2");

    lst.push_back("val_3");
    lst.push_front("val_4");
    REQUIRE(lst.get_size() == 2);
    REQUIRE(lst.at(0) == "val_4");
    REQUIRE(lst.pop_front() == "val_4");
    REQUIRE(lst.pop_back() == "val_3");
    REQUIRE(lst.get_size() == 0);

    REQUIRE(lst.pop_front().empty());
    REQUIRE(lst.pop_back().empty());

    lst.push_back("val_5");
    lst.push_back("val_6");
    lst.push_front("val_7");
    lst.push_front("val_8");
    REQUIRE(lst.at(1) == "val_7");
    REQUIRE(lst.get_size() == 4);

    REQUIRE(lst.pop_back() == "val_6");
    REQUIRE(lst.pop_front() == "val_8");
    REQUIRE(lst.pop_back() == "val_5");
    REQUIRE(lst.pop_front() == "val_7");
    REQUIRE(lst.get_size() == 0);
}

#endif

