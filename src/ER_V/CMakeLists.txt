# ER_V executable
add_executable(erv)

target_sources(erv PRIVATE
    main.cpp
    acl/acl.cpp
    erv_arm/erv.cpp
    erv_conf/erv_conf.cpp
    service.cpp
)

# Link libraries
target_link_libraries(erv PRIVATE
    common
    Threads::Threads
)
