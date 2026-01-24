# Driver library (subdirectory of Ichor)
add_library(driver OBJECT)

# C++ sources
target_sources(driver PRIVATE
    src/mcp/MCP23017.cpp
    src/motorhat/MotorHAT.cpp
    src/pwm/PCA9685.cpp
)

# Include directories
target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(driver PUBLIC
    common_objs
    Threads::Threads
)
