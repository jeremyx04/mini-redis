#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../src/resp.h"

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
