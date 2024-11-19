#include <iostream>
#include "resp.h"

int main(int argc, char **argv) {
    SimpleString simple_str("Hello World!");
    std::cout << simple_str.serialize();
    Error error("An error occurred...");
    std::cout << error.serialize();
    BulkString bulk_str("Bulk string!");
    std::cout << bulk_str.serialize();
    Integer x(22);
    std::cout << x.serialize();
    std::vector<std::unique_ptr<RType>> nested;
    nested.push_back(std::make_unique<Integer>(1));
    nested.push_back(std::make_unique<Integer>(2));
    std::unique_ptr<Array> nested_arr = std::make_unique<Array>(nested);

    std::vector<std::unique_ptr<RType>> vec;  
    vec.push_back(std::make_unique<SimpleString>("Element 0"));
    vec.push_back(std::move(nested_arr));
    Array arr(vec);

    std::cout << arr.serialize();
}
