# Common library
add_library(common STATIC)

target_sources(common PRIVATE
    log/log.c
    util/timestamp.c
    data/s_list.c
    api/api.cpp
    socket/socket.cpp
    sub/sub.cpp
    arm/arm.cpp
    conf/config.cpp
    log/log_config.cpp
    socket/socket_conf.cpp
)

# Include directories
target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../ER_V
)

# Link libraries
target_link_libraries(common PUBLIC
    Threads::Threads
)

# Set position independent code (good practice for static libraries)
set_target_properties(common PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
