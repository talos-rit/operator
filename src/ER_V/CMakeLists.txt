# ER_V executable
add_executable(erv)

# Main file
target_sources(erv PRIVATE
    main.cpp
)

# C sources
target_sources(erv PRIVATE
    acl/acl.c
)

# C++ sources
target_sources(erv PRIVATE
    erv_arm/erv.cpp
    erv_conf/erv_conf.cpp
)

# Link libraries
target_link_libraries(erv PRIVATE
    common
    Threads::Threads
)

# Tests
if(BUILD_TESTING)
    add_executable(erv_test)
    
    target_sources(erv_test PRIVATE
        all_tests.cpp
        acl/acl.c
        erv_arm/erv.cpp
        erv_conf/erv_conf.cpp
    )
    
    target_link_libraries(erv_test PRIVATE
        common
        Threads::Threads
        CppUTest::CppUTest
        CppUTest::CppUTestExt
    )
    
    add_test(NAME erv_test COMMAND erv_test)
endif()
