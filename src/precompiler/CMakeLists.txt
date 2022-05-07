

list(APPEND SOURCE_FILES precompiler.h)
list(APPEND SOURCE_FILES precompiler.cpp)

add_library(precompiler "${SOURCE_FILES}")

target_include_directories(precompiler PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(precompiler PUBLIC ${CMAKE_BINARY_DIR}/src)