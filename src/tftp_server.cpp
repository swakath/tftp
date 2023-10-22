/**
 * @file tftp_server.cpp
 * @brief TFTP Server.
 *
 * This file contains definations of function for TFTP Server side implementation as per RFC 1350
 *
 * @date October 21, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "include/tftp_server.hpp"

int initTFTPServer(){
    
}

void main(){
    // Create a UDP socket
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    // Server address structure
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT); // Server port
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr);

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (true) {
        // Receive data from the client
        ssize_t bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        buffer[bytesRead] = '\0';

        std::cout << "Received: " << buffer << std::endl;

        // Send the received data back to the client
        sendto(serverSocket, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientAddrLen);
    }

    // Close the socket
    close(serverSocket);

    return 0;
}
