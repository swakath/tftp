/**
 * @file tftp_client.cpp
 * @brief TFTP Client.
 *
 * This file contains definations for TFTP Client side implementation as per RFC 1350
 *
 * @date November 10, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "tftp_client.hpp"

/**
 * @brief Construct a new client Manager::client Manager object 
 */
clientManager::clientManager(){
    //
}

/**
 * @brief Function to set TFTP Client side communiction paramenters
 * 
 * @param rootDir 
 * @param fileName 
 * @param serverIP 
 * @param requestType 
 * @return true 
 * @return false 
 */
bool clientManager::commInit(std::string rootDir, std::string fileName, std::string serverIP, TftpOpcode requestType){
    if(requestType==TFTP_OPCODE_RRQ || requestType == TFTP_OPCODE_WRQ){
        this->root_dir = rootDir;
        this->requestFileName = fileName;
        this->requestType = requestType;
        this->blockNum = 0;
        this->operationMode = "octet"; // Currently only octet is supported
        
        this->defaultSocket = createRandomUDPSocket(clientIP, &this->portNumber);
        if(this->defaultSocket == -1){
            LOG(FATAL)<<"Error opening client side default socket";
            return false;
        }

        memset(&this->serverAddress, 0, sizeof(this->serverAddress));
        this->serverAddress.sin_family = AF_INET;  // using IPv4 address family
        if (inet_pton(AF_INET, serverIP.c_str(), &(this->serverAddress.sin_addr.s_addr)) !=1) {
            LOG(ERROR)<<"Error converting IP address: " << strerror(errno)<<" for port "<<this->portNumber;;
            return false;
        }
        this->serverAddress.sin_port = htons(TFTP_DEFAULT_PORT); // set tftp default server port
        return true;
    }
    else{
        LOG(ERROR)<<"Invalid request type";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}

/**
 * @brief Function to terminate TFTP Client communication
 * 
 */
void clientManager::commExit(){
    std::chrono::seconds duration(2);
    std::this_thread::sleep_for(duration);
    close(this->defaultSocket);
    return;
}

/**
 * @brief Function to handle tftp connection RRQ/WRQ requests
 * 
 */
void clientManager::handleTFTPConnection(){
    uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	int packetSize;
	if(this->requestType == TFTP_OPCODE_RRQ){
        LOG(INFO)<<"Read request process initatied";
        std::ofstream fd;
		TftpErrorCode errorCode;
		fd = STARK::getInstance().isFileWritable(this->requestFileName, errorCode);
		if(fd.is_open()){
            LOG(INFO)<<"Raw rile open success";
            bool ret;
            ret = handleReceiveData(fd);
            if(ret){
                LOG(INFO)<<"All data received";
            }
            else{
                LOG(ERROR)<<"All data not received";
                
            }
            ret = STARK::getInstance().closeWritableFile(this->requestFileName, fd);
            if(ret){
				LOG(INFO)<<"File Close Success";
			}
			else{
				LOG(ERROR)<<"File Close Error";
			}
            return;
        }
        else{
            LOG(ERROR)<<"Unable to open file";
            return;
        }
	}
	else if(this->requestType == TFTP_OPCODE_WRQ){
        LOG(INFO)<<"Write request process initiated";
        if(STARK::getInstance().isFileAvailable(this->requestFileName)){
            LOG(ERROR)<<"File Not Available in Disk";
            return;
        }
        std::ifstream fd;
		TftpErrorCode errorCode;
		fd = STARK::getInstance().isFileReadable(this->requestFileName, errorCode);
		if(fd.is_open()){
            LOG(INFO)<<"Raw rile open success";
            bool ret;
            ret = handleSendData(fd);
            if(ret){
                LOG(INFO)<<"All data Sent";
            }
            else{
                LOG(ERROR)<<"All data not Sent";

            }
            ret = STARK::getInstance().closeReadableFile(this->requestFileName, fd);
            if(ret){
				LOG(INFO)<<"File Close Success";
			}
			else{
				LOG(ERROR)<<"File Close Error";
			}
            return;
        }
        else{
            LOG(ERROR)<<"File open error";
            return;
        }
    }
	else{
        LOG(ERROR)<<"Invalid Opcode";
        return;
    }
	return;
}


/**
 * @brief Function to received data for TFTP Client
 * 
 */

bool clientManager::handleReceiveData(std::ofstream& fd){
    uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t recvData[TFTP_MAX_DATA_SIZE];
	uint16_t recvBlockNum = 0;
    
    if(fd.is_open()){
        bool allDataReceived = false;
		int sendPacketSize = 0;
		int ret = 0;
		bool dataRecvStatus = false;
		int recvDataLen = 0;
		int inValidTries = 0;
		bool isErrorPktReceived;
        bool isFirstPacket = true;

        sendPacketSize = makeComInitPacket(TFTP_OPCODE_RRQ,sendBuffer,sizeof(sendBuffer),this->requestFileName.c_str(),TFTP_MODE_OCTET);
        if(sendPacketSize == -1){
            LOG(ERROR)<<"unable to make data packet";
            return false;
        }
        ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
        if(ret != sendPacketSize){
            LOG(ERROR)<<"packet send error";
            return false;
        }

        while(!allDataReceived){
            sendPacketSize = 0;
			ret = 0;
			dataRecvStatus = false;
			recvDataLen = 0;
			isErrorPktReceived = false;

            if(inValidTries > TFTP_RECEIVE_TRIES){
				LOG(ERROR)<<"lost connection";
				return false;
			}
            dataRecvStatus = getData(this->defaultSocket,this->serverAddress, this->blockNum+1, recvData, sizeof(recvData), recvDataLen, isErrorPktReceived, isFirstPacket);

			if(dataRecvStatus){
				ret = writeData512(recvData, recvDataLen, fd);
				if(ret < 0){
					LOG(ERROR)<<"file write error";
                    sendPacketSize = 0;
                    sendPacketSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "Server side data write error");
			        sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
					return false;
				}
                if(isFirstPacket == true){
                    isFirstPacket = false;
                }
				this->blockNum++;
				inValidTries = 0;
				LOG(DEBUG)<< "receive data len: "<<recvDataLen<<" max:"<< TFTP_MAX_DATA_SIZE;
				if(recvDataLen < TFTP_MAX_DATA_SIZE){
					allDataReceived = true;
				}
			}
			else{
				if(!isErrorPktReceived){
					inValidTries++;
					LOG(ERROR)<<"Invalid data, soft continue";
				}
				else{
					LOG(ERROR)<<"Error received from client. Terminating transfer";
					return false;
				}
			}

            sendPacketSize = makeACKPacket(sendBuffer, sizeof(sendBuffer), this->blockNum);
			if(sendPacketSize == -1){
				LOG(ERROR)<<"unable to make data packet";
				return false;
			}
			ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
			if(ret != sendPacketSize){
				LOG(ERROR)<<"packet send error";
				return false;
			}
			LOG(DEBUG)<<"ACK "<<this->blockNum<<" sent to client";
        }
        if(allDataReceived){
            LOG(INFO)<<"All data received";
            return true;
        }else{
            LOG(ERROR)<<"Unexpected error";
            return false;
        } 

    }else{
        LOG(ERROR)<<"File open error";
        return false;
    }
}

/**
 * @brief Function to send data for TFTP Client
 * 
 * @param fd 
 * @return true 
 * @return false 
 */
bool handleSendData(std::ifstream& fd){

}