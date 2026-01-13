# src/ER_V/CMakeLists.txt
include_directories(
    ${CMAKE_SOURCE_DIR}/src/Ichor
)

# Ichor executable
set(ICHOR_CPP_SRCS
    arm/ichor_arm.cpp
    conf/ichor_conf.cpp
    main.cpp
)

add_executable(ichor
    ${ICHOR_CPP_SRCS}
)

add_subdirectory(driver)

target_link_libraries(ichor
    common_objs
    driver_objs
    Threads::Threads
)

# Install if needed
install(TARGETS ichor DESTINATION bin)