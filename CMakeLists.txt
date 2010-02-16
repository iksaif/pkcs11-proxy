cmake_minimum_required (VERSION 2.4)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project (pkcs11 C)

set(PKCS11_PROXY_SRCS gck-rpc-module.c gck-rpc-message.c gck-rpc-util.c egg-buffer.c)
set(PKCS11_DAEMON_SRCS egg-buffer.c gck-rpc-daemon-standalone.c gck-rpc-dispatch.c gck-rpc-message.c gck-rpc-util.c)

add_definitions(-Wall)
add_library(pkcs11-proxy SHARED ${PKCS11_PROXY_SRCS})
add_executable (pkcs11-daemon ${PKCS11_DAEMON_SRCS})

set_target_properties(pkcs11-proxy PROPERTIES VERSION 0.1 SOVERSION 0)

if (WIN32)
  include_directories("ext/include")

  add_library(dl STATIC IMPORTED)
  set_property(TARGET dl PROPERTY
               IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ext/lib/libdl.a)
  target_link_libraries (pkcs11-daemon ws2_32)
  target_link_libraries (pkcs11-proxy ws2_32)
endif (WIN32)

target_link_libraries (pkcs11-proxy pthread)
target_link_libraries (pkcs11-daemon dl pthread)

install_targets (/lib pkcs11-proxy)
install_targets (/bin pkcs11-daemon)
