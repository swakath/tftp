/**
 * @file tftp_socket.cpp
 * @brief TFTP Socket.
 *
 * This file contains definations of function for TFTP UDP Socket implementation as per RFC 1350
 *
 * @date November 3, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "tftp_socket.hpp"
/**
 * @brief function to create socket for the specified IP and PORT
*/
int createUDPSocket(const char* socketIP, int socketPORT, int timeOut){
    // socket file discriptor to be returned
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // checking if the socket creation succeeded
    if(sockfd == -1){
        LOG(ERROR)<<"Error creating socket: "<< strerror(errno)<<" for port "<<socketPORT;
		return -1;
    }

	struct timeval timeout;
    timeout.tv_sec = timeOut; // 5 seconds
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        LOG(ERROR)<<"Failed to set read timeout." << std::endl;
        return -1;
    }


	// clearing server address struct
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;  // using IPv4 address family

	if (inet_pton(AF_INET, socketIP, &(serv_addr.sin_addr.s_addr)) !=1) {
        LOG(ERROR)<<"Error converting IP address: " << strerror(errno)<<" for port "<<socketPORT;;
        return -1;
    }

	serv_addr.sin_port = htons(socketPORT); // set network port; if port=0 system determines the port

	// associate the socket with its local address
	int bound = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // checking if the socket bind succeeded
    if(bound == -1){
        LOG(ERROR)<<"Error binding socket: "<< strerror(errno)<<" for port "<<socketPORT;;
        close(sockfd);
        return -1;
    }

	// retrieve and print server formatted IP address xxx.xxx.xxx.xxx
	char ip[17];
	inet_ntop(serv_addr.sin_family, (void *)&serv_addr.sin_addr, ip, sizeof(ip));
     
	//Debug messages
	sprintf(log_message, "New Socket Created: IP [%s] Port [%d]", ip, ntohs(serv_addr.sin_port));
	LOG(DEBUG)<<""<< log_message;
	return sockfd;
}

/**
 * @brief creating udp socket with random available port 
*/
int createRandomUDPSocket(const char* socketIP, int* randomPort){
	if(socketIP != NULL && randomPort != NULL){
		int numTries = 0;
		int randomSocketFD;
		while(numTries < MAX_RANDOM_TRIES){
			srand(static_cast<unsigned int>(time(nullptr)));
			int curRandomPort = TFTP_MIN_PORT + (rand() % (TFTP_MAX_PORT - TFTP_MIN_PORT + 1));
			randomSocketFD = createUDPSocket(socketIP, curRandomPort);
			if(randomSocketFD != -1){
				*randomPort = curRandomPort;
				return randomSocketFD; 
			}
			numTries++;
		}
		return -1;
	}
	else {
		LOG(ERROR)<<"Input parameter error";
		return -1;
	}
}

/**
 * @brief send a uint8_t buffer to a client using udp
*/
int sendBufferThroughUDP(uint8_t* sendBuffer, size_t bufferLen, int socketfd, struct sockaddr_in clientAddress){
	if(sendBuffer!=NULL && bufferLen > 0 && socketfd > 0){
		size_t sendLen;
		socklen_t clientAddressLength = sizeof(clientAddress);
		sendLen = sendto(socketfd, sendBuffer, bufferLen, 0, (struct sockaddr*)&clientAddress, clientAddressLength);
		if(sendLen == -1){
			LOG(ERROR)<<"Data not send";
			return -1;
		}
		else if(sendLen != bufferLen){
			LOG(ERROR)<<"Full buffer not sent";
			return -1;
		}
		else{
			LOG(DEBUG)<<"Packet sent successfully";
			return sendLen;
		}
	}else{
		LOG(ERROR)<<"Input parameter error";
		return -1;
	}
	return -1;
}

/**
 * @brief function to read buffer length size of bytes from a UDP Socket receive buffer
*/
int getBufferThroughUDP(uint8_t* recvBuffer, size_t bufferLen, int socketfd, struct sockaddr_in& clientAddress){
	LOG(DEBUG)<<"recvBuffer Length: "<<bufferLen;
	if(recvBuffer!=NULL && bufferLen > 0){
		socklen_t clientAddressLength = sizeof(clientAddress);
		int timoutCnt = 0;
		while(timoutCnt < TFTP_MAX_TIMEOUT_TRIES){
			memset(recvBuffer, 0, bufferLen);
			ssize_t bytesReceived = recvfrom(socketfd, recvBuffer, bufferLen, 0, (struct sockaddr*)&clientAddress, &clientAddressLength);
			if (bytesReceived == -1) {
				if(errno == EWOULDBLOCK){
					timoutCnt++;
					LOG(ERROR)<<"timout occured count: "<<timoutCnt;
				}
				else{	
					LOG(ERROR)<<"Error receiving data "<<strerror(errno);
					return -1;
				}
			}
			else{
				LOG(DEBUG)<<"Received " << bytesReceived << " bytes from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " - Data:" << recvBuffer ;
				return (int)bytesReceived;
			}
		}
		if(timoutCnt >= TFTP_MAX_TIMEOUT_TRIES){
			LOG(ERROR)<<"MAX timeout occured";
			return -1;
		}
	}
	else{
		LOG(ERROR)<<"Input parameter error";
		return -1;
	}
	return -1;
}


/**
 * @brief function to handle receiveing ACK from a TFTP Client
*/
bool getACK(int clientSocket, struct sockaddr_in clientAddress, uint16_t expectedBlockNum){
	uint8_t recvBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	int packetSize;
	
	int ret = 0;
	struct sockaddr_in recvAddress;
	ret = getBufferThroughUDP(recvBuffer, sizeof(recvBuffer), clientSocket, recvAddress);
	if(ret == -1){
		LOG(ERROR)<<"receive error";
		return false;
	}

	if(recvAddress.sin_addr.s_addr!=clientAddress.sin_addr.s_addr){ 
		packetSize = 0;
		LOG(ERROR)<<"packet from unknown host";
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NO_SUCH_USER, "you are a unknow user");
		sendBufferThroughUDP(sendBuffer, packetSize, clientSocket, recvAddress);
		return false;
	}

	if(recvAddress.sin_port!=clientAddress.sin_port){
		packetSize = 0;
		LOG(ERROR)<<"invalid TID";
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_UNKNOWN_TID, "you are a unknow user");
		sendBufferThroughUDP(sendBuffer, packetSize, clientSocket, recvAddress);
		return false;
	}

	if(ret < 4){
		LOG(ERROR)<<"invalid packet expected ACK paket of 4 bytes";
		return false;
	}

	uint16_t opcode = TFTP_OPCODE_ND;
	// retriving opcode
	opcode = (uint16_t)(((recvBuffer[1] & 0xFF) << 8) | (recvBuffer[0] & 0XFF));
	opcode = ntohs(opcode);
	if(opcode != TFTP_OPCODE_ACK){
		LOG(ERROR)<<"invalid opcode expected ACK";
		return false;
	}

	uint16_t recvBlockNum = 0;
	// retriving block number
	recvBlockNum = (uint16_t)(((recvBuffer[3] & 0xFF) << 8) | (recvBuffer[2] & 0XFF));
	recvBlockNum = ntohs(recvBlockNum);
	if(recvBlockNum != expectedBlockNum){
		LOG(ERROR)<<"invalid block number";
		return false;
	}
	LOG(DEBUG)<<"Valid ACK Received block number: "<<recvBlockNum;
	return true;
}

/**
 * @brief receives TFTP data packet from specified client socket
*/
bool getData(int clientSocket, struct sockaddr_in clientAddress, uint16_t expectedBlockNum, uint8_t* recvDataBuffer, size_t bufferSize, int& dataLen){
	uint8_t recvBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	int packetSize = 0;
	if(recvDataBuffer!=NULL){
		memset(recvDataBuffer,0,bufferSize);
		dataLen = 0;
		int ret = 0;
		struct sockaddr_in recvAddress;
		ret = getBufferThroughUDP(recvBuffer, sizeof(recvBuffer), clientSocket, recvAddress);
		LOG(DEBUG)<<"receive buffer length "<<ret;
		if(ret == -1){
			LOG(ERROR)<<"receive error";
			return false;
		}

		if(recvAddress.sin_addr.s_addr!=clientAddress.sin_addr.s_addr){ 
			packetSize = 0;
			LOG(ERROR)<<"packet from unknown host";
			packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NO_SUCH_USER, "you are a unknow user");
			sendBufferThroughUDP(sendBuffer, packetSize, clientSocket, recvAddress);
			return false;
		}

		if(recvAddress.sin_port!=clientAddress.sin_port){
			packetSize = 0;
			LOG(ERROR)<<"invalid TID";
			packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_UNKNOWN_TID, "you are a unknow user");
			sendBufferThroughUDP(sendBuffer, packetSize, clientSocket, recvAddress);
			return false;
		}

		if(ret < 4){
			LOG(ERROR)<<"invalid packet expected data paket 4bytes or greater";
			return false;
		}

		uint16_t opcode = TFTP_OPCODE_ND;
		// retriving opcode
		opcode = (uint16_t)(((recvBuffer[1] & 0xFF) << 8) | (recvBuffer[0] & 0XFF));
		opcode = ntohs(opcode);

		// Verifying data opcode
		if(opcode != TFTP_OPCODE_DATA){
			LOG(ERROR)<<"invalid opcode expected DATA";
			return false;
		}

		uint16_t recvBlockNum = 0;
		// retriving block number
		recvBlockNum = (uint16_t)(((recvBuffer[3] & 0xFF) << 8) | (recvBuffer[2] & 0XFF));
		recvBlockNum = ntohs(recvBlockNum);

		if(recvBlockNum != expectedBlockNum){
			LOG(ERROR)<<"invalid block number"<<", received:"<<recvBlockNum<<", expected:"<<expectedBlockNum;
			return false;
		}

		dataLen = ret - 4;
		if(dataLen > TFTP_MAX_DATA_SIZE){
			LOG(ERROR)<<"invalid data size";
			return false;
		}
		if(dataLen > 0){
			memcpy(recvDataBuffer, recvBuffer + 4, dataLen);
		}

		LOG(DEBUG)<<"Valid Data Received block number: "<<recvBlockNum<<", Length: "<<dataLen;
		return true;
	}
	else{
		LOG(ERROR)<<"invalid funciton argument";
		return false;
	}
	return false;
}