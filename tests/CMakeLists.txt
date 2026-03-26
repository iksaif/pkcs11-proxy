add_executable(test_egg_buffer test_egg_buffer.c ../egg-buffer.c)
target_include_directories(test_egg_buffer PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME egg_buffer COMMAND test_egg_buffer)

add_executable(test_rpc_message test_rpc_message.c ../egg-buffer.c ../gck-rpc-message.c ../gck-rpc-util.c)
target_include_directories(test_rpc_message PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME rpc_message COMMAND test_rpc_message)
