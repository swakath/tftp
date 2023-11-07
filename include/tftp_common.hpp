/**
 * @file tftp_common.hpp
 * @brief Common definitions and functions for the TFTP module.
 *
 * This header file contains common definitions, constants, and utility functions
 * used throughout the TFTP module implementation. 
 *
 * @date October 21, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#ifndef COMM_H
#define COMM_H

#include <iostream>
#include <cstdlib>

#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdarg>
#include <cstdint>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>

#include <string>
#include <vector>

#ifndef EASYLOGGINGPP_H
    #include "easylogging++.h"
#endif

#define TFTP_DEFAULT_PORT 69
#define TFTP_MAX_DATA_SIZE 512
#define TFTP_MAX_HEADER_SIZE 4
#define TFTP_MAX_PACKET_SIZE (TFTP_MAX_DATA_SIZE + TFTP_MAX_HEADER_SIZE)
#define TFTP_MAX_MODE_SIZE 9
#define TFTP_MIN_CONN_INIT_PACKET_SIZE 8 //8 bytes minimum rrq/wrq packet size
#define TFTP_MIN_PORT 1024
#define TFTP_MAX_PORT 65535

#define LOG_BUFF_SIZE 1024

static char log_message[LOG_BUFF_SIZE];
static const char* TFTP_MODE_OCTET = "octet";


/**
* @brief TFTP opcodes as per RFC 1350
*/
typedef enum : uint16_t
{
    TFTP_OPCODE_ND    = 0, // Not Defined 
    TFTP_OPCODE_RRQ   = 1, // Read request
    TFTP_OPCODE_WRQ   = 2, // Write request
    TFTP_OPCODE_DATA  = 3, // Data
    TFTP_OPCODE_ACK   = 4, // Acknowledgment
    TFTP_OPCODE_ERROR = 5 // Error
} TftpOpcode;
  
  
/**
* @brief TFTP error codes as per  RFC 1350 
*/
typedef enum : uint16_t
{
    TFTP_ERROR_NOT_DEFINED         = 0,
    TFTP_ERROR_FILE_NOT_FOUND      = 1,
    TFTP_ERROR_ACCESS_VIOLATION    = 2,
    TFTP_ERROR_DISK_FULL           = 3,
    TFTP_ERROR_ILLEGAL_OPERATION   = 4,
    TFTP_ERROR_UNKNOWN_TID         = 5,
    TFTP_ERROR_FILE_ALREADY_EXISTS = 6,
    TFTP_ERROR_NO_SUCH_USER        = 7
} TftpErrorCode;

#endif