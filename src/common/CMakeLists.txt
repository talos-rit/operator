# src/common/CMakeLists.txt

add_library(common_objs OBJECT)

# Treat C sources as C++ since we are in the process of converting them
# set_source_files_properties(${COMMON_C_SRCS} PROPERTIES LANGUAGE CXX)

target_sources(common_objs
    PRIVATE
        log/log.cpp
        util/timestamp.cpp
        data/s_list.cpp
        api/api.cpp
        socket/socket.cpp
        sub/sub.cpp
        arm/arm.cpp
        conf/config.cpp
        log/log_config.cpp
        socket/socket_conf.cpp
)

target_include_directories(common_objs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common_objs PUBLIC Threads::Threads)

set(COMMON_TARGET common_objs PARENT_SCOPE)
