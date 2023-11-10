cmake_minimum_required(VERSION 3.1)
project(unitTests VERSION 1.0 LANGUAGES C CXX)

find_package(GTest REQUIRED)
find_package (Threads)

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
            "${TEST_SRC_DIR}/main.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE ${CODE_INCLUED_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${OUTPUT_DIR}")
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_compile_definitions(${PROJECT_NAME} PRIVATE ELPP_THREAD_SAFE ELPP_FRESH_LOG_FILE)
target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}) 

#file(COPY "testFiles" DESTINATION "${OUTPUT_DIR}")

enable_testing()
add_test(add ${PROJECT_NAME})