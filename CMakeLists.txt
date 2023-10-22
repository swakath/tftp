cmake_minimum_required(VERSION 3.10)

project(tftp VERSION 1.0 LANGUAGES C CXX)

# Specify the directories where to find header files
include_directories(include)

set(SOURCES
    src/easylogging++.cc
    main.cpp
)

# Create an executable from the source files
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ELPP_THREAD_SAFE ELPP_FRESH_LOG_FILE)