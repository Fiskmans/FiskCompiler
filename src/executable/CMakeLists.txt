
list(APPEND SOURCE_FILES main.cpp)

add_executable(fiskCompiler "${SOURCE_FILES}")

target_include_directories(fiskCompiler PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(fiskCompiler PUBLIC ${CMAKE_BINARY_DIR}/src)

add_dependencies(fiskCompiler tools)
add_dependencies(fiskCompiler tokenizer)