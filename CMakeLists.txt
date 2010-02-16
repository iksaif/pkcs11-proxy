cmake_minimum_required (VERSION 2.6)

project (pkcs11 C)

add_definitions(-Wall)
add_library(pkcs11-proxy.so SHARED gck-rpc-module.c gck-rpc-message.c egg-buffer.c)
add_executable (pkcs11-daemon egg-buffer.c gck-rpc-daemon-standalone.c gck-rpc-dispatch.c gck-rpc-message.c gck-rpc-util.c)
target_link_libraries (pkcs11-daemon dl pthread)
