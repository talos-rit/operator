# src/common/CMakeLists.txt
# C Sources
set(COMMON_C_SRCS
    log/log.c
    util/timestamp.c
    data/s_list.c
)

# C++ Sources
set(COMMON_CPP_SRCS
    api/api.cpp
    socket/socket.cpp
    sub/sub.cpp
    arm/arm.cpp
    conf/config.cpp
    log/log_config.cpp
    socket/socket_conf.cpp
)

add_library(common_objs OBJECT)

# Treat C sources as C++ since we are in the process of converting them
set_source_files_properties(${COMMON_C_SRCS} PROPERTIES LANGUAGE CXX)

target_sources(common_objs
    PRIVATE
        ${COMMON_C_SRCS}
        ${COMMON_CPP_SRCS}
)

target_include_directories(common_objs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common_objs PUBLIC Threads::Threads)

set(COMMON_TARGET common_objs PARENT_SCOPE)

