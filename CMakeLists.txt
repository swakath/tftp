cmake_minimum_required(VERSION 3.10)

project(tftp VERSION 1.0 LANGUAGES C CXX)

find_package(Threads REQUIRED)

# Specify the directories where to find header files
include_directories(include)

 
set(COMMON_SOURCES
    src/easylogging++.cc
    src/tftp_socket.cpp
    src/tftp_packets.cpp
    src/tftp_stark.cpp
    src/tftp_server.cpp
    src/tftp_client.cpp
    src/huffman.cpp
)

# Create an executable for server
add_executable(tftpServer 
            ${COMMON_SOURCES}
            src/server_main.cpp
            )

# Create an executable for server
add_executable(tftpClient 
            ${COMMON_SOURCES}
            src/client_main.cpp
            )

# add_executable(testHuffman
#             ${COMMON_SOURCES}
#             src/main_huffman.cpp
#             )

target_link_libraries(tftpServer Threads::Threads stdc++fs)
target_compile_definitions(tftpServer PRIVATE ELPP_THREAD_SAFE ELPP_FRESH_LOG_FILE)

target_link_libraries(tftpClient Threads::Threads stdc++fs)
target_compile_definitions(tftpClient PRIVATE ELPP_THREAD_SAFE ELPP_FRESH_LOG_FILE)

# target_link_libraries(testHuffman Threads::Threads stdc++fs)
# target_compile_definitions(testHuffman PRIVATE ELPP_THREAD_SAFE ELPP_FRESH_LOG_FILE)