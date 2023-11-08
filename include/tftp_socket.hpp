/**
 * @file tftp_socket.hpp
 * @brief TFTP Socket.
 *
 * This file contains prototypes and constants for TFTP UDP Socket related implementation as per RFC 1350
 *
 * @date November 3, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#ifndef TFTP_SOCK
#define TFTP_SOCK

#ifndef COMM_H
    #include "tftp_common.hpp"
#endif

#define TFTP_UDP_TIMEOUT 10 //10 seconds
#define TFTP_MAX_TIMEOUT_TRIES 3
#define MAX_RANDOM_TRIES 3

int createUDPSocket(const char* socketIP, int socketPORT, int timeOut = TFTP_UDP_TIMEOUT);
int createRandomUDPSocket(const char* socketIP, int* randomPort);
int sendBufferThroughUDP(uint8_t* sendBuffer, size_t bufferLen, int socketfd, struct sockaddr_in clientAddress);
int getBufferThroughUDP(uint8_t* recvBuffer, size_t bufferLen, int socketfd, struct sockaddr_in& clientAddress);
#endif