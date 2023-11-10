/**
 * @file tftp_client.hpp
 * @brief TFTP Client.
 *
 * This file contains prototypes and constants for TFTP Client side implementation as per RFC 1350
 *
 * @date November 10, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#ifndef TFTP_CLIENT
#define TFTP_CLIENT

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

#ifndef TFTP_STARK_H
    #include "tftp_stark.hpp"
#endif

#define CLIENT_READ "READ"  //RRQ CLI
#define CLIENT_WRITE "WRITE" //WRQ CLI
#define CLIENT_DELETE "DELETE" //DEL CLI
#define TFTP_RECEIVE_TRIES 3
#define TFTP_CLIENT_SOCKET_TIMEOUT 1800

static char clientDir[TFTP_MAX_DATA_SIZE] = "/home/swakath/tftpClient/";
static char clientIP[16] = "127.0.0.7";

class clientManager : public Singleton<clientManager>{
    friend class Singleton<clientManager>;
    protected:
        clientManager();
    public:
        std::string root_dir;
        int defaultSocket; // UDP Socket File Discriptorser
        int portNumber; // UDP Port number
        struct sockaddr_in serverAddress;
        TftpOpcode requestType; //RRQ or WRQ
        std::string requestFileName;
        std::string compressedFile;
        uint16_t blockNum; // Last block number sent or received
        std::string operationMode; // Currently operates only in octate mode
        bool commInit(std::string rootDir, std::string fileName, std::string serverIP, TftpOpcode requestType);
        void commExit();
        void handleTFTPConnection();
        bool handleReceiveData(std::ofstream& fd);
        bool handleSendData(std::ifstream& fd);
};
#endif