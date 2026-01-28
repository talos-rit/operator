# Ichor object library (linkable by tests/executables)
add_library(ichor_objs OBJECT)
    
target_sources(ichor_objs
    PRIVATE
        arm/ichor_arm.cpp
        conf/ichor_conf.cpp
)

target_include_directories(ichor_objs 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link shared dependencies to the object lib
target_link_libraries(ichor_objs PUBLIC
    common_objs
    driver
    Threads::Threads
)

# Ichor executable (still builds as before)
add_executable(ichor)

target_sources(ichor PRIVATE
    main.cpp
)

target_link_libraries(ichor PRIVATE 
    common_objs
    driver    
    ichor_objs
    Threads::Threads
)

add_subdirectory(driver)
