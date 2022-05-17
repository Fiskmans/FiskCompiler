

list(APPEND SOURCE_FILES token.h)
list(APPEND SOURCE_FILES token.cpp)
list(APPEND SOURCE_FILES tokenizer.h)
list(APPEND SOURCE_FILES tokenizer.cpp)
list(APPEND SOURCE_FILES tokenMatcher.h)
list(APPEND SOURCE_FILES tokenMatcher.cpp)
list(APPEND SOURCE_FILES tokenStream.h)
list(APPEND SOURCE_FILES tokenStream.cpp)

add_library(tokenizer "${SOURCE_FILES}")

target_include_directories(tokenizer PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(tokenizer PUBLIC ${CMAKE_BINARY_DIR}/src)