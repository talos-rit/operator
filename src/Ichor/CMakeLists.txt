# src/Ichor/CMakeLists.txt
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


target_include_directories(ichor PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Ichor
    ${CMAKE_SOURCE_DIR}/src/common
)

add_subdirectory(driver)

target_include_directories(ichor PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Ichor/driver/src
)

target_link_libraries(ichor PRIVATE
    common_objs
    driver
    Threads::Threads
)

# # Install if needed
# install(TARGETS ichor DESTINATION bin)