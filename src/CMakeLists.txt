add_library(resp_impl resp.cpp)
add_library(store_impl store.cpp)
add_library(redis_impl redisengine.cpp)

target_include_directories(resp_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(store_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(redis_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
