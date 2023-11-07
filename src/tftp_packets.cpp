/**
 * @file tftp_packets.hpp
 * @brief function defination for the TFTP standard packet generation.
 *
 * @date November 7, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "tftp_packets.hpp"

/**
 * @brief function accepts error codes and generates the error packet corresponding to it.
*/
int makeErrorPacket(uint8_t* sendBuffer, size_t bufferLen, TftpErrorCode errorCode, const char* msgError){
    int indx = 0;
    if((sendBuffer != NULL) && (bufferLen >= 5) && (errorCode >= TFTP_ERROR_NOT_DEFINED || errorCode <= TFTP_ERROR_NO_SUCH_USER)){
        memset(sendBuffer, 0, bufferLen);
        // Copying opcode to the error packet
        uint16_t networkOpcode = htons(TFTP_OPCODE_ERROR);
        sendBuffer[indx] = (uint8_t)(networkOpcode & 0xFF);
        sendBuffer[indx+1] = (uint8_t)(networkOpcode>>8 & 0xFF);
        indx += 2;

        // Copying error code to the error packet
        uint16_t networkErrorCode = htons(errorCode);
        sendBuffer[indx] = (uint8_t)(networkErrorCode & 0xFF);
        sendBuffer[indx+1] = (uint8_t)((networkErrorCode>>8) & 0xFF);
        indx += 2;
        
        // Copying error message to error packet
        if(bufferLen > 5){
            size_t cpyLen = std::min(strlen(msgError), bufferLen - 5);
            memcpy(sendBuffer+indx, msgError, cpyLen);
            indx += cpyLen;
        }

        // Adding end of packet 
        sendBuffer[indx] = 0x00;
        indx++;
        return indx;    
    }
    return -1;
}