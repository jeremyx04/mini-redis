#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../src/resp.h"
#include <memory>

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
