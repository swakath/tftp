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
    if(requestType==TFTP_OPCODE_RRQ || requestType == TFTP_OPCODE_WRQ || requestType == TFTP_OPCODE_DEL){
        this->root_dir = rootDir;
        this->requestFileName = fileName;
        this->requestType = requestType;
        this->blockNum = 0;
        this->operationMode = "octet"; // Currently only octet is supported
        this->compObj.setRootDir(rootDir);
        this->compObj.setFileName(fileName);
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
        if(STARK::getInstance().isFileAvailable(this->requestFileName)){
            LOG(ERROR)<<"File already available in disk";
            return;
        }
        // Remove to integrate compression.
		//fd = STARK::getInstance().isFileWritable(this->requestFileName, errorCode);
		fd = STARK::getInstance().isFileWritable(this->compObj.compressFileName, errorCode);
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
            ret = STARK::getInstance().closeWritableFile(this->compObj.compressFileName, fd);
            if(ret){
				LOG(INFO)<<"File Close Success";
			}
			else{
				LOG(ERROR)<<"File Close Error";
                return;
			}

            ret = this->compObj.decompressFile();
            if(ret){
                LOG(INFO)<<"Decompression successful";
            }
            else{
                LOG(ERROR)<<"Error decompressing the received file";
                return;
            }
            TftpErrorCode dummy;
            //ret = STARK::getInstance().isFileDeletable(this->compObj.compressFileName, dummy);
            if(ret){
                LOG(INFO)<<"All temp files deleted";
            }
            else{
                LOG(ERROR)<<"Error while deleting temp files";
                return;
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
        if(!STARK::getInstance().isFileAvailable(this->requestFileName)){
            LOG(ERROR)<<"File Not Available in Disk";
            return;
        }
        std::ifstream fd;
		TftpErrorCode errorCode;
        // Remove to integrate compression.
		//fd = STARK::getInstance().isFileReadable(this->requestFileName, errorCode);
		bool compRet;
        compRet = this->compObj.compressFile();
        if(!compRet){
            LOG(ERROR)<<"Error when compressing file";
            return;
        }
        LOG(INFO)<<"Compression success";

        fd = STARK::getInstance().isFileReadable(this->compObj.compressFileName, errorCode);
		
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
            ret = STARK::getInstance().closeReadableFile(this->compObj.compressFileName, fd);
            if(ret){
				LOG(INFO)<<"File Close Success";
			}
			else{
				LOG(ERROR)<<"File Close Error";
			}
            TftpErrorCode dummy;
            ret = true;
            //ret = STARK::getInstance().isFileDeletable(this->compObj.compressFileName, dummy);
            if(ret){
				LOG(INFO)<<"Temp files deleted";
			}
			else{
				LOG(ERROR)<<"Temp file deletion error";
			}
            return;
        }
        else{
            LOG(ERROR)<<"File open error";
            return;
        }
    }
    else if(this->requestType == TFTP_OPCODE_DEL){
        LOG(INFO)<<"Delete request process initatied";
        int sendPacketSize = 0;
        int ret = 0;
        sendPacketSize = makeComInitPacket(TFTP_OPCODE_DEL,sendBuffer,sizeof(sendBuffer),this->requestFileName.c_str(),TFTP_MODE_OCTET);
        if(sendPacketSize < 0){
            LOG(ERROR)<<"Error Generating Delete Packet";
            return;
        }
        ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
        if(ret != sendPacketSize){
            LOG(ERROR)<<"packet send error";
            return;
        }
        bool recvError = false;
        bool isValidAck = false;
        for(int getAckTries = 0; getAckTries < TFTP_RECEIVE_TRIES; ++getAckTries){
            recvError = false;
            isValidAck = false;
            isValidAck =  getACK(this->defaultSocket, this->serverAddress, TFTP_VALID_DELETE_ACK, recvError, true);
            if(isValidAck){
                LOG(INFO)<<"File Deletion success";
                return;
            }
            if(!recvError){
                LOG(ERROR)<<"No responce, soft continue";
            }else{
                LOG(ERROR)<<"Error received";
                return;
            }
        }
        LOG(ERROR)<<"No valid ACK received from server. Terminating connection";
        sendPacketSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "no ack received. time out error");
        sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
        return;
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
		bool isErrorPktReceived = false;
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
    LOG(ERROR)<<"Open Condition";
    return false;
}

/**
 * @brief Function to send data for TFTP Client
 * 
 * @param fd 
 * @return true 
 * @return false 
 */
bool clientManager::handleSendData(std::ifstream& fd){
    uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t dataBuffer[TFTP_MAX_DATA_SIZE];
	if(fd.is_open()){
		bool allDataSent = false;
		int bytesRead = 0;
		int sendPacketSize = 0;
		int ret = 0;
		bool ackStatus = false;
		int inValidTries = 0;
		int getNewPacket = true;
		bool isErrorPktReceived = false;
        bool isFirstACKReceived = false;
        sendPacketSize = makeComInitPacket(TFTP_OPCODE_WRQ,sendBuffer,sizeof(sendBuffer),this->requestFileName.c_str(),TFTP_MODE_OCTET);
        if(sendPacketSize == -1){
            LOG(ERROR)<<"unable to make data packet";
            return false;
        }
        ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
        if(ret != sendPacketSize){
            LOG(ERROR)<<"packet send error";
            return false;
        }

        while(!allDataSent){
			if(inValidTries > TFTP_RECEIVE_TRIES){
				LOG(ERROR)<<"lost connection";
				return false;
			}
			ret = 0;
			sendPacketSize = 0;
			ackStatus = false;
			isErrorPktReceived = false;
            
            ackStatus = getACK(this->defaultSocket, this->serverAddress, this->blockNum, isErrorPktReceived, !isFirstACKReceived);
			if(ackStatus){
				LOG(DEBUG)<<"Valid ACK received";
				inValidTries = 0;
				getNewPacket = true;
				LOG(DEBUG)<<"Bytes read and sent: "<<bytesRead;
				if(bytesRead < TFTP_MAX_DATA_SIZE && isFirstACKReceived){
					LOG(DEBUG)<<"All data sent";
					allDataSent = true;
                    break;
				}
                if(isFirstACKReceived == false){
                    isFirstACKReceived = true;
                }
			}
			else{
				if(!isErrorPktReceived){
					LOG(ERROR)<<"Invalid ack, soft continue";
					getNewPacket = false;
					inValidTries++;
				}
				else{
					LOG(ERROR)<<"Error received from client. Terminating transfer";
					return false;
				}
			}

            if(isFirstACKReceived){
                if(getNewPacket){
                    bytesRead = 0;
                    bytesRead = readData512(dataBuffer, sizeof(dataBuffer), fd);
                    if(bytesRead == -1){
                        LOG(ERROR)<<"file read error";
                        sendPacketSize = 0;
                        sendPacketSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "Server side data read error");
			            sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
                        return false;
                    }
                    this->blockNum++;
                }
                sendPacketSize = makeDataPacket(sendBuffer, sizeof(sendBuffer), this->blockNum, dataBuffer, bytesRead);
                if(sendPacketSize == -1){
                    LOG(ERROR)<<"unable to make data packet";
                    return false;
                }
                LOG(DEBUG)<<"Data packet generated successfully";
                ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, this->defaultSocket, this->serverAddress);
                if(ret != sendPacketSize){
                    LOG(ERROR)<<"packet send error";
                    return false;
                }
                LOG(DEBUG)<<"Data packet sent successfully";
            }
            else{
                LOG(ERROR)<<"First ACK not received. Soft continue.";
            }
        }
        if(allDataSent){
            LOG(INFO)<<"All data sent";
            return true;
        }
        else{
            LOG(ERROR)<<"Unexpected error";
            return false;
        }
    }
    else{
        LOG(ERROR)<<"File open error";
        return false;
    }
    LOG(ERROR)<<"Open Condition";
    return false;
}