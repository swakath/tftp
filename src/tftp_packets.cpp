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
int makeErrorPacket(uint8_t* sendBuffer, size_t bufferLen, uint16_t errorCode, const char* msgError){
    int indx = 0;
    if((sendBuffer != NULL) && (bufferLen >= 5) && (errorCode >= TFTP_ERROR_NOT_DEFINED || errorCode <= TFTP_ERROR_NO_SUCH_USER)){
        memset(sendBuffer, 0, bufferLen);
        sendBuffer[indx] = (uint8_t)(TFTP_OPCODE_ERROR & 0xFF);
        sendBuffer[indx+1] = (uint8_t)((TFTP_OPCODE_ERROR>>8) & 0xFF);
        indx += 2;

        sendBuffer[indx] = (uint8_t)(errorCode & 0xFF);
        sendBuffer[indx+1] = (uint8_t)((errorCode>>8) & 0xFF);
        indx += 2;
        if(bufferLen > 5){
            size_t cpyLen = std::min(strlen(msgError), bufferLen - 5);
            memcpy(sendBuffer+indx, msgError, cpyLen);
            indx += cpyLen;
        }
        sendBuffer[indx] = 0x00;
        indx++;
        return indx;    
    }
    return -1;
}