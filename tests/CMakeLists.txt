include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
)

FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(tatlin_tests unit_tests.cpp)

target_include_directories(tatlin_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(tatlin_tests
        PRIVATE
        tatlin_lib
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(tatlin_tests)
