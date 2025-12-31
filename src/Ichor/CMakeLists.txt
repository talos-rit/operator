# Add driver subdirectory first
add_subdirectory(driver)

# Ichor executable
add_executable(ichor)

# Main file
target_sources(ichor PRIVATE
    main.cpp
)

# C++ sources
target_sources(ichor PRIVATE
    arm/ichor_arm.cpp
    conf/ichor_conf.cpp
)

# Include directories
target_include_directories(ichor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(ichor PRIVATE
    common
    driver
    Threads::Threads
)

# Dummy executable
add_executable(dummy)

target_sources(dummy PRIVATE
    dummy_main.cpp
)

target_sources(dummy PRIVATE
    arm/ichor_arm.cpp
    conf/ichor_conf.cpp
)

target_include_directories(dummy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dummy PRIVATE
    common
    driver
    Threads::Threads
)

# Tests
if(BUILD_TESTING)
    add_executable(ichor_test)
    
    target_sources(ichor_test PRIVATE
        all_tests.cpp
        arm/ichor_arm.cpp
        conf/ichor_conf.cpp
    )
    
    target_include_directories(ichor_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    target_link_libraries(ichor_test PRIVATE
        common
        driver
        Threads::Threads
        CppUTest::CppUTest
        CppUTest::CppUTestExt
    )
    
    add_test(NAME ichor_test COMMAND ichor_test)
endif()
