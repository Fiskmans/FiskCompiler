

list(APPEND SOURCE_FILES fileHelpers.cpp)
list(APPEND SOURCE_FILES fileHelpers.h)

add_library(tools "${SOURCE_FILES}")

target_include_directories(tools PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(tools PUBLIC ${CMAKE_BINARY_DIR}/src)