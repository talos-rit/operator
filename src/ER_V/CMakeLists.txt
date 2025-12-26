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
    common
    Threads::Threads
)

# Tests
if(BUILD_TESTING)
    add_executable(erv_test)
    
    target_sources(erv_test PRIVATE
        all_tests.cpp
        acl/acl.cpp
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
