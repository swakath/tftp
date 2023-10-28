/**
 * @file tftp_server.cpp
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

#ifndef COMM_H
    #include "common.hpp"
#endif

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#define DEFAULT_PORT 69
const char* serverIP = "127.0.0.1";
const char* END_SERVER_MSG = "END_SERVER";
bool END_SERVER_PROCESS = false;
int createUDPSocket(const char* socketIP, int socketPORT);

#endif