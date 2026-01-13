# src/ER_V/CMakeLists.txt
include_directories(${CMAKE_SOURCE_DIR}/src/ER_V)

# C sources
set(ERV_C_SRCS
    acl/acl.c
)
# C++ sources
set(ERV_CPP_SRCS
    main.cpp
    erv_arm/erv.cpp
    erv_conf/erv_conf.cpp
)

# Treat C sources as C++ since we are in the process of converting them
set_source_files_properties(${ERV_C_SRCS} PROPERTIES LANGUAGE CXX)

add_executable(erv
    ${ERV_C_SRCS}
    ${ERV_CPP_SRCS}
)

target_link_libraries(erv
    common_objs
    Threads::Threads
)

# Install if needed
install(TARGETS erv DESTINATION bin)