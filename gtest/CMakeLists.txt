cmake_minimum_required(VERSION 3.1)
project(unitTests VERSION 1.0 LANGUAGES C CXX)

# Enable testing
enable_testing()

# Check if coverage option is enabled
option(CODE_COVERAGE "Enable code coverage with gcov and lcov" OFF)

# Find necessary packages
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Set directories
set(TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CODE_SRC_DIR "${TEST_SRC_DIR}/../src")
set(CODE_INCLUED_DIR "${TEST_SRC_DIR}/../include")
set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Specify the directories where to find header files
include_directories(include)

set(SOURCES
    ${CODE_SRC_DIR}/easylogging++.cc
    ${CODE_SRC_DIR}/tftp_socket.cpp
    ${CODE_SRC_DIR}/tftp_packets.cpp
    ${CODE_SRC_DIR}/tftp_stark.cpp
    ${CODE_SRC_DIR}/tftp_server.cpp
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}  
    "${TEST_SRC_DIR}/packetMaking.cpp"
    "${TEST_SRC_DIR}/main.cpp"
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CODE_INCLUED_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${OUTPUT_DIR}")

# Link libraries
target_link_libraries(${PROJECT_NAME} Threads::Threads gmock stdc++fs)
target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Define preprocessor macros
target_compile_definitions(${PROJECT_NAME} PRIVATE ELPP_THREAD_SAFE ELPP_FRESH_LOG_FILE)

# Code coverage settings
if(CODE_COVERAGE)
    message(STATUS "Code coverage enabled")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -O0)
        target_link_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -lgcov)
    endif()

    # Add custom target to generate coverage report
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${GENHTML_PATH} coverage.info --output-directory coverage_report
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated at ${CMAKE_BINARY_DIR}/coverage_report/index.html"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report"
        )
    else()
        message(WARNING "lcov or genhtml not found, coverage report won't be generated")
    endif()
endif()

# Copy test files to output directory
file(COPY "testfiles" DESTINATION "${OUTPUT_DIR}")

# Add test to CTest
add_test(add ${PROJECT_NAME})