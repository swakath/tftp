/**
 * @file tftp_server.hpp
 * @brief TFTP Server.
 *
 * This file contains prototypes and constants for TFTP Server side implementation as per RFC 1350
 *
 * @date October 21, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#ifndef TFTP_SER
#define TFTP_SER

#ifndef TFTP_SOCK
    #include "tftp_socket.hpp"
#endif

#ifndef TFTP_PACK_H
    #include "tftp_packets.hpp"
#endif

#ifndef COMM_H
    #include "tftp_common.hpp"
#endif

static char serverIP[16] = "127.0.0.1";
static const char* END_SERVER_MSG = "END_SERVER";
static bool END_SERVER_PROCESS = false;

class ClientHandler {     
    public:
        int defaultServerSocket;
        int clientSocket;
        struct sockaddr_in clientAddress;
        uint16_t requestType; //RRQ or WRQ
        char requestFileName[TFTP_MAX_DATA_SIZE];
        char operationMode[TFTP_MAX_MODE_SIZE]; // Currently operates only in octate mode

        ClientHandler();
        ClientHandler(int defaultServerSocket, sockaddr_in clientAddress, uint16_t requestType, char* requestFileName, char* operationMode);
        void printVals();
};

void handleIncommingRequests(int serverSock);
void handleServerTermination();
void handleClient(ClientHandler curClient);

#endif