#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../src/resp.h"

// sanity check... 
TEST_CASE("Math works!") {
    REQUIRE(2+2 == 4); 
}

TEST_CASE("Serialize simple string") {
    SimpleString simple_str("Hello World!");
    std::string serialized = simple_str.serialize();
    REQUIRE(serialized == "+Hello World!\r\n");
}
