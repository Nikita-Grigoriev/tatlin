add_executable(tatlin_app main.cpp)

target_include_directories(tatlin_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(tatlin_app PRIVATE tatlin_lib)
