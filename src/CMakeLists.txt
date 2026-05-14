add_library(tatlin_lib STATIC
        FileTape.cpp
        FileTapeFactory.cpp
        TapeSorter.cpp
        Config.cpp
)

target_include_directories(tatlin_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
