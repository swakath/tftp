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

#ifndef TFTP_SERVER
#define TFTP_SERVER

#ifndef TFTP_SOCK
    #include "tftp_socket.hpp"
#endif

#ifndef TFTP_PACK_H
    #include "tftp_packets.hpp"
#endif

#ifndef COMM_H
    #include "tftp_common.hpp"
#endif

#ifndef TFTP_STARK_H
    #include "tftp_stark.hpp"
#endif

#define TFTP_RECEIVE_TRIES 3
#define TFTP_SERVER_SOCKET_TIMEOUT 1800
static char serverIP[16] = "127.0.0.1";
static char serverDir[TFTP_MAX_DATA_SIZE] = "/home/swakath/tftpRoot/";
static const char* END_SERVER_MSG = "END_SERVER";
static bool END_SERVER_PROCESS = false;

class ClientHandler {     
    public:
        int defaultServerSocket;
        int clientSocket;
        struct sockaddr_in clientAddress;
        uint16_t requestType; //RRQ or WRQ
        std::string requestFileName;
        uint16_t blockNum; // Last block number sent or received
        char operationMode[TFTP_MAX_MODE_SIZE]; // Currently operates only in octate mode
        ClientHandler();
        ClientHandler(int defaultServerSocket, sockaddr_in clientAddress, uint16_t requestType, char* requestFileName, char* operationMode);
        void printVals();
};

void handleClient(ClientHandler curClient);
void handleIncommingRequests(int serverSock);
void handleServerTermination();
bool handleSendData(ClientHandler curClient, std::ifstream& fd);
bool handleReceiveData(ClientHandler curClient, std::ofstream& fd);
void closeSocket(int socketFD); 
bool list_dir(std::string dir, std::string&  fname);
#endif