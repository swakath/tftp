/**
 * @file tftp_packets.hpp
 * @brief function prototypes for the TFTP standard packet generation.
 *
 * @date November 7, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 
#ifndef TFTP_PACK_H
#define TFTP_PACK_H

#ifndef COMM_H
    #include "tftp_common.hpp"
#endif

int makeErrorPacket(uint8_t* sendBuffer, size_t bufferLen, TftpErrorCode errorCode, const char* msgError);

#endif
