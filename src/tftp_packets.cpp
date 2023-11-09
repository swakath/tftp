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
 * @brief function accepts TFTP error codes and generates the corresponding error packet.
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

/**
 * @brief function accept RRQ or WRQ opcode, file name, and operation mode. It generates RRQ/WRQ packet for TFTP as per RFC 1350
*/
int makeComInitPacket(TftpOpcode opcode, uint8_t* sendBuffer, size_t bufferLen, const char* fileName, const char* mode){
    int indx = 0;
    if((sendBuffer != NULL) && (fileName != NULL) && (mode != NULL) && (opcode == TFTP_OPCODE_RRQ || opcode == TFTP_OPCODE_WRQ)){
        size_t fileNameLen = strlen(fileName);
        size_t modeLen = strlen(fileName);
        // Send Buffer underflow condition verification
        if(bufferLen < 4 + fileNameLen + modeLen){
            return -1;
        }
        memset(sendBuffer, 0, bufferLen);
        // Copying RRQ/WRQ opcode to the packet
        uint16_t networkOpcode = htons(opcode);
        sendBuffer[indx] = (uint8_t)(networkOpcode & 0xFF);
        sendBuffer[indx+1] = (uint8_t)(networkOpcode>>8 & 0xFF);
        indx += 2;
        // Copying file name to the packet
        memcpy(sendBuffer+indx, fileName, fileNameLen);
        indx += fileNameLen;
        // Including file name termination byte
        sendBuffer[indx] = 0x00;
        indx++;
        // Copying mode to the packet
        memcpy(sendBuffer+indx, mode, modeLen);
        indx += modeLen;
        //Including mode name termination byte
        sendBuffer[indx] = 0x00;
        indx++;
        return indx;
    }
    else{
        return -1;
    }
    return -1;
}

/**
 * @brief function accepts Block number and generates a TFTP ACK packet
*/
int makeACKPacket(uint8_t* sendBuffer, size_t bufferLen, uint16_t blockNum){
    int indx = 0;
    if(sendBuffer!=NULL && (bufferLen>=4)){
        memset(sendBuffer, 0, bufferLen);
        // Copying opcode to the ACK packet
        uint16_t networkOpcode = htons(TFTP_OPCODE_ACK);
        sendBuffer[indx] = (uint8_t)(networkOpcode & 0xFF);
        sendBuffer[indx+1] = (uint8_t)(networkOpcode>>8 & 0xFF);
        indx += 2;
        // Copying Block number to ACK packet
        uint16_t networkBlockNum = htons(blockNum);
        sendBuffer[indx] = (uint8_t)(networkBlockNum & 0xFF);
        sendBuffer[indx+1] = (uint8_t)((networkBlockNum>>8) & 0xFF);
        indx += 2;
        return indx;
    }
    else{
        return -1;
    }
    return -1;
}

/**
 * @brief function accepts Block number and Data buffer and generates a TFTP Data packet
*/
int makeDataPacket(uint8_t* sendBuffer, size_t bufferLen, uint16_t blockNum, uint8_t* data, size_t dataLen){
    int indx = 0;
    if(sendBuffer!=NULL && data!=NULL){
        if(dataLen > TFTP_MAX_DATA_SIZE){
            return -1;
        }
        if(bufferLen < dataLen + 4){
            return -1;
        }
        memset(sendBuffer, 0, bufferLen);
        // Copying opcode to the DATA Packet
        uint16_t networkOpcode = htons(TFTP_OPCODE_DATA);
        sendBuffer[indx] = (uint8_t)(networkOpcode & 0xFF);
        sendBuffer[indx+1] = (uint8_t)(networkOpcode>>8 & 0xFF);
        indx += 2;
        // Copying Block number to Data packet
        uint16_t networkBlockNum = htons(blockNum);
        sendBuffer[indx] = (uint8_t)(networkBlockNum & 0xFF);
        sendBuffer[indx+1] = (uint8_t)((networkBlockNum>>8) & 0xFF);
        indx += 2;
        // Copying Data to Data packet
        if(dataLen > 0){
            memcpy(sendBuffer+indx, data, dataLen);
            indx += dataLen;
        }
        return indx;
    }
    else{
        return -1;
    }
    return -1;
}

/**
 * @brief function reads maximum 512 bytes of data from a ifstream file are copies the data into a buffer 
*/

int readData512(uint8_t* dataBuffer, size_t bufferLen, std::ifstream& fd){
    if(dataBuffer!=NULL && bufferLen >= TFTP_MAX_DATA_SIZE && fd.is_open()){
        memset(dataBuffer, 0, bufferLen);
        int bytesRead;
        if(!fd.eof()){
            fd.read(reinterpret_cast<char*>(dataBuffer), bufferLen);
            if(fd.fail()){
                LOG(ERROR)<<"file read error";
                return -1;
            }
            bytesRead = static_cast<int>(fd.gcount());
            LOG(DEBUG)<<bytesRead<<" bytes read successful";
            return bytesRead;
        }
        else{
            LOG(DEBUG)<<"eof retruned 0 bytes read successful";
            return 0;
        }
    }
    else{
        LOG(ERROR)<<"Input argument error";
        return -1;
    }
    return -1;
}

/**
 * @brief function writes maximum 512 bytes from a data buffer to a ofstream file 
*/
int writeData512(uint8_t* dataBuffer, size_t bufferLen, std::ofstream& fd){
    if(dataBuffer!=NULL && bufferLen <= TFTP_MAX_DATA_SIZE && fd.is_open()){
        fd.write(reinterpret_cast<char*>(dataBuffer), bufferLen);
        if(fd.fail()){
            LOG(ERROR)<<"file write error";
            return -1;
        }
        LOG(DEBUG)<<"file write success";
        return 1;
    }
    else{
        LOG(ERROR)<<"Input argument error"<<dataBuffer<<","<<bufferLen<<","<<fd.is_open();
        return -1;
    }
    return -1;
}