# ER_V object library (linkable by tests/executables)
add_library(erv_objs OBJECT)

target_sources(erv_objs
    PRIVATE
        acl/acl.cpp
        erv_arm/erv.cpp
        erv_conf/erv_conf.cpp
)

target_include_directories(erv_objs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(erv_objs PUBLIC
    common_objs
    Threads::Threads
)

# ER_V executable
add_executable(erv)

target_sources(erv PRIVATE
        main.cpp
)


# Link libraries
target_link_libraries(erv PRIVATE
    common_objs
    erv_objs
    Threads::Threads
)
