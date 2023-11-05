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

int createUDPSocket(const char* socketIP, int socketPORT);

#endif