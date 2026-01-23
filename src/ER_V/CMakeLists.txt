# ER_V executable
add_executable(erv)

target_sources(erv PRIVATE
    main.cpp
    acl/acl.cpp
    erv_arm/erv.cpp
    erv_conf/erv_conf.cpp
)


# Link libraries
target_link_libraries(erv PRIVATE
    common_objs
    Threads::Threads
)

target_include_directories(erv PRIVATE
    ${CMAKE_SOURCE_DIR}/src/ER_V
)