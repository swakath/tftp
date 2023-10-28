/**
 * @file common.hpp
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

#define LOG_BUFF_SIZE 1024
char log_message[LOG_BUFF_SIZE];
#endif