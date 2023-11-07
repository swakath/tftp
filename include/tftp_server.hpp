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

#ifndef SINGLETON_H
    #include "singleton.hpp"
#endif

static char serverIP[16] = "127.0.0.1";
static char serverDir[TFTP_MAX_DATA_SIZE] = "/mnt/swakath/tftpRoot/";
static const char* END_SERVER_MSG = "END_SERVER";
static bool END_SERVER_PROCESS = false;

class ClientHandler {     
    public:
        int defaultServerSocket;
        int clientSocket;
        struct sockaddr_in clientAddress;
        uint16_t requestType; //RRQ or WRQ
        std::string requestFileName;
        char operationMode[TFTP_MAX_MODE_SIZE]; // Currently operates only in octate mode

        ClientHandler();
        ClientHandler(int defaultServerSocket, sockaddr_in clientAddress, uint16_t requestType, char* requestFileName, char* operationMode);
        void printVals();
};
 
 /**
 * @brief "STARK" stands for "Simple Tracker for Accessing and Recording for file Keeping" 
 * Singleton class to manage all the file access previlages  
*/
class STARK : public Singleton<STARK> {
    friend class Singleton<STARK>;
    protected:
        STARK();
        std::mutex mutexObj;
    public:
        std::string root_dir;
        std::unordered_map<std::string, std::pair<int, bool>> fileData;
        void setRootDir(const char* directory);
        std::ifstream isFileReadable(std::string fileName, TftpErrorCode& errorCode);
        std::ofstream isFileWritable(std::string fileName, TftpErrorCode& errorCode);
        bool closeReadableFile(std::string fileName, std::ifstream fd);
        bool closeWritableFile(std::string fileName, std::ofstream fd);
};

void handleIncommingRequests(int serverSock);
void handleServerTermination();
void handleClient(ClientHandler curClient);
#endif