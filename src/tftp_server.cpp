/**
 * @file tftp_server.cpp
 * @brief TFTP Server.
 *
 * This file contains definations of function for TFTP Server side implementation as per RFC 1350
 *
 * @date October 21, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "tftp_server.hpp"

/**
 * @brief constructor for ClientHandler Class
*/
ClientHandler::ClientHandler(){
	defaultServerSocket = 0;
	clientSocket = 0;
	requestType = 0;
	requestFileName = "";
	blockNum = 0;
	//Currently only OCTET mode is supported
	strcpy(operationMode, TFTP_MODE_OCTET);
}

/**
 * @brief parameterized constructor for ClientHandler Class
*/

ClientHandler::ClientHandler(int defaultServerSocket, sockaddr_in clientAddress, uint16_t requestType, char* requestFileName, char* operationMode){
	this->defaultServerSocket = defaultServerSocket;
	this->clientAddress = clientAddress;
	this->requestType = requestType;
	this->requestFileName.assign(requestFileName);
	// Currently only OCTET mode is supported
	strcpy(this->operationMode , operationMode);
	blockNum = 0;
}

/**
 * @brief debug print funtion for ClientHander Class
*/
void ClientHandler::printVals(){
	memset(log_message,0,sizeof(log_message));
	sprintf(log_message, "Client Obj Vals: IP[%s] Port[%d] fileName[%s] mode[%s]", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), requestFileName.c_str(), operationMode);
	LOG(INFO) << log_message;
	return;
}

/**
 * @brief function to handle incomming client request to default TFTP server port
*/
void handleIncommingRequests(int serverSock){
	
	char recvBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	int packetSize;
    uint16_t opcode;
	char fileName[TFTP_MAX_DATA_SIZE];
	char mode[TFTP_MAX_MODE_SIZE];
	std::vector<ClientHandler> clientObjects;
	std::vector<std::thread> clientThreads;

	while (!END_SERVER_PROCESS) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
		memset(recvBuffer, 0, sizeof(recvBuffer));

        ssize_t bytesReceived = recvfrom(serverSock, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr*)&clientAddress, &clientAddressLength);

        if (bytesReceived == -1) {
            LOG(ERROR) <<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ", msg: Error receiving data: " << strerror(errno);
            continue;
        }

		LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ", msg: Received " << bytesReceived << " bytes from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " - Data:" << recvBuffer ;
		
		// Checking sanity of recvBuffer
		if(sizeof(recvBuffer) <= TFTP_MIN_CONN_INIT_PACKET_SIZE*sizeof(uint8_t)){
			LOG(ERROR) <<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ", msg: Invalid request in port: "<<TFTP_DEFAULT_PORT;
			continue;
		}

		opcode = TFTP_OPCODE_ND;
		memset(fileName, 0, sizeof(fileName));
		memset(mode, 0, sizeof(mode));

		// retriving opcode
		opcode = (uint16_t)(((recvBuffer[1] & 0xFF) << 8) | (recvBuffer[0] & 0XFF));
		sprintf(log_message, "recv [%X][%X] raw opcode [%X]", recvBuffer[0], recvBuffer[1], opcode);
		LOG(INFO)<<log_message;	
		opcode = ntohs(opcode);

		if(opcode!=TFTP_OPCODE_RRQ && opcode!=TFTP_OPCODE_WRQ){
			packetSize = 0;
			LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ",msg: Recv"<<opcode<<", Comp"<<TFTP_OPCODE_RRQ<<":"<<TFTP_OPCODE_WRQ;
			LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ", msg: Incompatable OPCODE received from "<< inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port);
			packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ILLEGAL_OPERATION, "opcode invalid");
			sendBufferThroughUDP(sendBuffer, packetSize, serverSock, clientAddress);
			continue;
		}
		
		// retriving file name
		strcpy(fileName, (char*)(recvBuffer+2));

		// retriving communication mode
		strcpy(mode,(char*)(recvBuffer + 2 + strlen(fileName) + 1));
		
		for (int i = 0; mode[i] != '\0'; i++) {
        	mode[i] = std::tolower(mode[i]);
		}

		if(std::strcmp(TFTP_MODE_OCTET, mode)!=0){
			packetSize = 0;
			LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ", msg: Offset value" << 2+strlen(fileName)+1;
			LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<< ", msg: Incompatable MODE received from "<< inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port)<<"mode: "<<mode;
			packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ILLEGAL_OPERATION, "incompatable mode");
			sendBufferThroughUDP(sendBuffer, packetSize, serverSock, clientAddress);
			continue;
		}

		memset(log_message,0,sizeof(log_message));
		sprintf(log_message, "Connection request received: IP[%s] Port[%d] fileName[%s] mode[%s]", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fileName, mode);
		LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: "<<log_message;

		ClientHandler curClientHandlerObj(serverSock ,clientAddress, opcode, fileName, mode);
		clientObjects.push_back(curClientHandlerObj);
		curClientHandlerObj.printVals();
		std::thread curClientHandle(handleClient, curClientHandlerObj);
		int a = 10;
		//std::thread curClientHandle(test, a);
		clientThreads.push_back(std::move(curClientHandle));
	}

	for(int i = 0 ; i<clientThreads.size(); ++i){
		clientThreads[i].join();
	}
	return;
}

/**
 * @brief function handles the terminate of server process
*/
void handleServerTermination(){
	std::string userInput;
	while(!END_SERVER_PROCESS){
		std::cout<<"ENTER END_SERVER INPUT TO TERMINATE THE SERVER: ";
		std::cin>>userInput;
		if(strncmp(userInput.c_str(), END_SERVER_MSG, strlen(END_SERVER_MSG)) == 0){
			END_SERVER_PROCESS = true;
		} 
		else {
			std::cout<<"INVALID INPUT\n";	
		}
	}
	return;
}

void handleClient(ClientHandler curClient){
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	int packetSize;
	int clientPort = 0;
	int clientSocketFD = 0;
	clientSocketFD = createRandomUDPSocket(serverIP, &clientPort);
	
	if(clientSocketFD == -1){
		packetSize = 0;
		LOG(ERROR)<<"Unable to create UPD Socket for client: IP"<<inet_ntoa(curClient.clientAddress.sin_addr) << ": PORT" << ntohs(curClient.clientAddress.sin_port);
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unable to create new socket");
		sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
		return;
	}
	curClient.clientSocket = clientSocketFD;
    LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: new port"<<clientPort<<" new fd:"<<clientSocketFD<<"default fd: "<<curClient.defaultServerSocket;
	if(curClient.requestType == TFTP_OPCODE_RRQ){
		LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: Read request process initiated";
		std::ifstream fd;
		TftpErrorCode errorCode; 
		fd = STARK::getInstance().isFileReadable(curClient.requestFileName, errorCode);
		if(fd.is_open()){
			LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Open Success";
			bool ret;
			ret = handleSendData(curClient, fd);
			if(ret){
				LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: All data sent";
			}
			else{
				packetSize = 0;
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: Data not sent";
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "error connection terminating");
				sendBufferThroughUDP(sendBuffer, packetSize, curClient.clientSocket, curClient.clientAddress);
			}
			ret = STARK::getInstance().closeReadableFile(curClient.requestFileName, fd);
			if(ret){
				LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Close Success";
			}
			else{
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Close Error";
			}
			close(clientSocketFD);
			return;
		}
		else{
			packetSize = 0;
			if(errorCode == TFTP_ERROR_ACCESS_VIOLATION){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ACCESS_VIOLATION, "file opened in write mode by another client");
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: file opened in write mode by another client";
			}
			else if(errorCode == TFTP_ERROR_FILE_NOT_FOUND){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_FILE_NOT_FOUND, "file not found in server");
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: file not found in server";
			}
			else{
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unknown error from server");
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: unknown error from server";
			}
			sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
			close(clientSocketFD);
			return;
		}
	}
	else if(curClient.requestType == TFTP_OPCODE_WRQ){
		LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: Write request process initiated";
		std::ofstream fd;
		TftpErrorCode errorCode;
		fd = STARK::getInstance().isFileWritable(curClient.requestFileName, errorCode);
		if(fd.is_open()){
			//curClient.fdRead = std::move(fd);
			LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Open Success";
			LOG(DEBUG)<<"fd status:"<<fd.is_open();
			std::string debg= "I am GOD";
			fd << debg;
			LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Content: "<<debg;
			
			bool ret;
			ret = STARK::getInstance().closeWritableFile(curClient.requestFileName, fd);
			if(ret){
				LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Close Success";
			}
			else{
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<",msg: File Close Error";
			}
			close(clientSocketFD);
			return;
		}
		else{
			packetSize = 0;
			if(errorCode == TFTP_ERROR_FILE_ALREADY_EXISTS){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_FILE_ALREADY_EXISTS, "file already exists in server");
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: file already exists in server";
			}
			else if(errorCode == TFTP_ERROR_ACCESS_VIOLATION){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ACCESS_VIOLATION, "file access denied in server");
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: file access denied in server";
			}
			else{
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unknown error from server");
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: unknown error from server";
			}
			sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
			close(clientSocketFD);
			return;
		}
	}
	else{
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: invalid opcode";
		packetSize  = 0;
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "invalid opcode during internal processing");
		sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
		close(clientSocketFD);
		return;
	}
	close(clientSocketFD);
	return;
}

/**
 * @brief function to handle RRQ task for a specific client.
*/
bool handleSendData(ClientHandler curClient, std::ifstream& fd){
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
		while(!allDataSent){
			if(inValidTries > TFTP_RECEIVE_TRIES){
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: lost connection";
				return false;
			}
			bytesRead = 0;
			ret = 0;
			sendPacketSize = 0;
			ackStatus = false;
			if(getNewPacket){
				bytesRead = readData512(dataBuffer, sizeof(dataBuffer), fd);
				if(bytesRead == -1){
					LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: file read error";
					return false;
				}
				curClient.blockNum++;
			}
			sendPacketSize = makeDataPacket(sendBuffer, sizeof(sendBuffer), curClient.blockNum, dataBuffer, bytesRead);
			if(sendPacketSize == -1){
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: unable to make data packet";
				return false;
			}
			ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, curClient.clientSocket, curClient.clientAddress);
			if(ret != sendPacketSize){
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: packet send error";
				return false;
			}
			ackStatus = getACK(curClient);
			if(ackStatus){
				inValidTries = 0;
				getNewPacket = true;
				if(bytesRead < TFTP_MAX_DATA_SIZE){
					allDataSent = true;
				}
			}
			else{
				getNewPacket = false;
				inValidTries++;
			}	
		}
		if(allDataSent){
			LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: all data sent to client";
			return true;
		}
		return false;
	}
	else{
		return false;
	}
	return false;
}

bool getACK(ClientHandler curClient){
	uint8_t recvBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
	int packetSize;
	
	int ret = 0;
	struct sockaddr_in recvAddress;
	ret = getBufferThroughUDP(recvBuffer, sizeof(recvBuffer), curClient.clientSocket, recvAddress);
	if(ret == -1){
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: receive error";
		return false;
	}

	if(recvAddress.sin_addr.s_addr!=curClient.clientAddress.sin_addr.s_addr){ 
		packetSize = 0;
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: packet from unknown host";
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NO_SUCH_USER, "you are a unknow user");
		sendBufferThroughUDP(sendBuffer, packetSize, curClient.clientSocket, recvAddress);
		return false;
	}

	if(recvAddress.sin_port!=curClient.clientAddress.sin_port){
		packetSize = 0;
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: invalid TID";
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_UNKNOWN_TID, "you are a unknow user");
		sendBufferThroughUDP(sendBuffer, packetSize, curClient.clientSocket, recvAddress);
		return false;
	}
		
	uint16_t opcode = TFTP_OPCODE_ND;
	// retriving opcode
	opcode = (uint16_t)(((recvBuffer[1] & 0xFF) << 8) | (recvBuffer[0] & 0XFF));
	opcode = ntohs(opcode);
	if(opcode != TFTP_OPCODE_ACK){
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: invalid opcode expected ACK";
		return false;
	}

	uint16_t recvBlockNum;
	// retriving block number
	recvBlockNum = (uint16_t)(((recvBuffer[3] & 0xFF) << 8) | (recvBuffer[2] & 0XFF));
	recvBlockNum = ntohs(recvBlockNum);
	if(recvBlockNum != curClient.blockNum){
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: invalid block number";
		return false;
	}
	LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<"msg: Valid ACK Received block number: "<<recvBlockNum;
	return true;
}