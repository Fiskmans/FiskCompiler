
list(APPEND SOURCE_FILES main.cpp)

add_executable(fiskCompiler "${SOURCE_FILES}")

target_include_directories(fiskCompiler PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(fiskCompiler PUBLIC ${CMAKE_BINARY_DIR}/src)

target_link_libraries(fiskCompiler PUBLIC tokenizer)
target_link_libraries(fiskCompiler PUBLIC tools)
target_link_libraries(fiskCompiler PUBLIC common)