find_package(Catch2 3 QUIET)
if(Catch2_FOUND)
    message(STATUS "Using packaged version of Catch")
else()
    message(STATUS "Downloading bundled version of catch")
    include(FetchContent)
    FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG 53d0d913a422d356b23dd927547febdf69ee9081 
    )
    FetchContent_MakeAvailable(Catch2)
endif()

add_executable(redis_tests redis.test.cpp)

target_link_libraries(redis_tests PRIVATE Catch2::Catch2WithMain linkedlist_impl resp_impl store_impl redis_impl)

include(Catch)  
catch_discover_tests(redis_tests)
