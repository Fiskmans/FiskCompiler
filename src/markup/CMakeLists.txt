
list(APPEND SOURCE_FILES Pattern.h Pattern.cpp)
list(APPEND SOURCE_FILES Patterns.h Patterns.cpp)

add_library(markup "${SOURCE_FILES}")

target_include_directories(markup PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(markup PUBLIC ${CMAKE_BINARY_DIR}/src)