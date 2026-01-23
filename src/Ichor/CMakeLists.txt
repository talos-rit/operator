# ichor executable
add_executable(ichor)

target_sources(ichor PRIVATE
    main.cpp
    arm/ichor_arm.cpp
    conf/ichor_conf.cpp
)

# Link libraries
target_link_libraries(ichor PRIVATE
    common_objs
    driver
    Threads::Threads
)

target_include_directories(ichor PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Ichor
)

add_subdirectory(driver)