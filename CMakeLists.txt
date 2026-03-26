cmake_minimum_required(VERSION 3.10)

project(pkcs11-proxy VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" ON)

# Common source lists
set(COMMON_SRCS gck-rpc-message.c gck-rpc-util.c egg-buffer.c)
set(PKCS11_PROXY_SRCS gck-rpc-module.c ${COMMON_SRCS})
set(PKCS11_DAEMON_SRCS gck-rpc-daemon-standalone.c gck-rpc-dispatch.c ${COMMON_SRCS})

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

# --- pkcs11-proxy shared library ---
add_library(pkcs11-proxy SHARED ${PKCS11_PROXY_SRCS})
target_include_directories(pkcs11-proxy PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(pkcs11-proxy PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# --- pkcs11-daemon executable ---
# Disable console when building Win32 binary in release mode
if(WIN32 AND CMAKE_BUILD_TYPE MATCHES "^Rel.*")
    set(GUI_TYPE WIN32)
endif()

add_executable(pkcs11-daemon ${GUI_TYPE} ${PKCS11_DAEMON_SRCS})
target_include_directories(pkcs11-daemon PRIVATE ${CMAKE_SOURCE_DIR})

# --- Platform-specific linking ---
if(WIN32)
    target_include_directories(pkcs11-proxy PRIVATE ext/include)
    target_include_directories(pkcs11-daemon PRIVATE ext/include)

    add_library(dl STATIC IMPORTED)
    set_property(TARGET dl PROPERTY
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ext/lib/libdl.a)

    target_link_libraries(pkcs11-daemon PRIVATE ws2_32)
    target_link_libraries(pkcs11-proxy PRIVATE ws2_32)
endif()

find_package(Threads REQUIRED)
target_link_libraries(pkcs11-proxy PRIVATE Threads::Threads)
target_link_libraries(pkcs11-daemon PRIVATE ${CMAKE_DL_LIBS} Threads::Threads)

# --- Install ---
include(GNUInstallDirs)
install(TARGETS pkcs11-proxy LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS pkcs11-daemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# --- Tests ---
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
