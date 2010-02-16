cmake_minimum_required (VERSION 2.6)

project (pkcs11 C)

set(PKCS11_PROXY_SRCS gck-rpc-module.c gck-rpc-message.c gck-rpc-util.c egg-buffer.c)
set(PKCS11_DAEMON_SRCS egg-buffer.c gck-rpc-daemon-standalone.c gck-rpc-dispatch.c gck-rpc-message.c gck-rpc-util.c)

add_definitions(-Wall)
add_library(pkcs11-proxy SHARED ${PKCS11_PROXY_SRCS})
add_executable (pkcs11-daemon ${PKCS11_DAEMON_SRCS})

if (WIN32)
  include_directories("ext/include")

  add_library(dl STATIC IMPORTED)
  set_property(TARGET dl PROPERTY
               IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ext/lib/libdl.a)
  target_link_libraries (pkcs11-daemon ws2_32)
  target_link_libraries (pkcs11-proxy ws2_32)
endif ()

target_link_libraries (pkcs11-proxy pthread)
target_link_libraries (pkcs11-daemon dl pthread)
