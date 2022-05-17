
list(APPEND SOURCE_FILES main.cpp)

add_executable(fiskCompiler "${SOURCE_FILES}")

target_include_directories(fiskCompiler PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(fiskCompiler PUBLIC ${CMAKE_BINARY_DIR}/src)

target_link_libraries(fiskCompiler PUBLIC tokenizer)
target_link_libraries(fiskCompiler PUBLIC tools)
target_link_libraries(fiskCompiler PUBLIC common)
target_link_libraries(fiskCompiler PUBLIC precompiler)

#add_custom_command(TARGET fiskCompiler POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                       ${PROJECT_BINARY_DIR}\..\std\*.cpp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\$<CONFIGURATION>\std\)

set(STD_LIBRARY_FOLDER ${CMAKE_SOURCE_DIR}/src/standard_libraries)
set(DATA_FOLDER ${CMAKE_SOURCE_DIR}/data)
set(TARGET_FOLDER $<TARGET_FILE_DIR:fiskCompiler>)

message(std folder: ${STD_LIBRARY_FOLDER})
message(data folder: ${DATA_FOLDER})
message(target: ${TARGET_FOLDER})

add_custom_command(TARGET fiskCompiler POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${STD_LIBRARY_FOLDER} ${TARGET_FOLDER})
				   
add_custom_command(TARGET fiskCompiler POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${DATA_FOLDER} ${TARGET_FOLDER})