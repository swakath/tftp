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
int makeComInitPacket(TftpOpcode opcode,uint8_t* sendBuffer, size_t bufferLen, const char* fileName, const char* mode);
int makeACKPacket(uint8_t* sendBuffer, size_t bufferLen, uint16_t blockNum);
int makeDataPacket(uint8_t* sendBuffer, size_t bufferLen, uint16_t blockNum, uint8_t* data, size_t dataLen);
int readData512(uint8_t* dataBuffer, size_t bufferLen, std::ifstream& fd);
#endif
