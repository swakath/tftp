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
	LOG(DEBUG) << log_message;
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
            LOG(ERROR) <<"Error receiving data: "<< strerror(errno);
            continue;
        }

		LOG(DEBUG)<<"Received " << bytesReceived << " bytes from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " - Data:" << recvBuffer ;
		
		// Checking sanity of recvBuffer
		if(sizeof(recvBuffer) <= TFTP_MIN_CONN_INIT_PACKET_SIZE*sizeof(uint8_t)){
			LOG(ERROR) << "Invalid request in port: "<<TFTP_DEFAULT_PORT;
			continue;
		}

		opcode = TFTP_OPCODE_ND;
		memset(fileName, 0, sizeof(fileName));
		memset(mode, 0, sizeof(mode));

		// retriving opcode
		opcode = (uint16_t)(((recvBuffer[1] & 0xFF) << 8) | (recvBuffer[0] & 0XFF));
		sprintf(log_message, "recv [%X][%X] raw opcode [%X]", recvBuffer[0], recvBuffer[1], opcode);
		LOG(DEBUG)<<log_message;	
		opcode = ntohs(opcode);

		if(opcode!=TFTP_OPCODE_RRQ && opcode!=TFTP_OPCODE_WRQ && opcode!=TFTP_OPCODE_DEL){
			packetSize = 0;
			LOG(ERROR)<< "Recv"<<opcode<<", Comp"<<TFTP_OPCODE_RRQ<<":"<<TFTP_OPCODE_WRQ;
			LOG(ERROR)<< "Incompatable OPCODE received from "<< inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port);
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
			LOG(ERROR)<< "Offset value" << 2+strlen(fileName)+1;
			LOG(ERROR)<< "Incompatable MODE received from "<< inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port)<<"mode: "<<mode;
			packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ILLEGAL_OPERATION, "incompatable mode");
			sendBufferThroughUDP(sendBuffer, packetSize, serverSock, clientAddress);
			continue;
		}

		memset(log_message,0,sizeof(log_message));
		sprintf(log_message, "Connection request received: IP[%s] Port[%d] fileName[%s] mode[%s]", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fileName, mode);
		LOG(INFO)<<log_message;

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
    LOG(INFO)<<"new port"<<clientPort<<" new fd:"<<clientSocketFD<<"default fd: "<<curClient.defaultServerSocket;
	if(curClient.requestType == TFTP_OPCODE_RRQ){
		LOG(INFO)<<"Read request process initiated";
		std::ifstream fd;
		TftpErrorCode errorCode; 
		fd = STARK::getInstance().isFileReadable(curClient.requestFileName, errorCode);
		if(fd.is_open()){
			LOG(DEBUG)<<"File Open Success";
			bool ret;
			ret = handleSendData(curClient, fd);
			if(ret){
				LOG(INFO)<<"All data sent";
			}
			else{
				packetSize = 0;
				LOG(ERROR)<<"Data not sent";
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "error connection terminating");
				sendBufferThroughUDP(sendBuffer, packetSize, curClient.clientSocket, curClient.clientAddress);
			}
			ret = STARK::getInstance().closeReadableFile(curClient.requestFileName, fd);
			if(ret){
				LOG(INFO)<<"File Close Success";
			}
			else{
				LOG(ERROR)<<"File Close Error";
			}
			closeSocket(clientSocketFD);
			return;
		}
		else{
			packetSize = 0;
			if(errorCode == TFTP_ERROR_ACCESS_VIOLATION){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ACCESS_VIOLATION, "file opened in write mode by another client");
				LOG(ERROR)<<"file opened in write mode by another client";
			}
			else if(errorCode == TFTP_ERROR_FILE_NOT_FOUND){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_FILE_NOT_FOUND, "file not found in server");
				LOG(ERROR)<<"file not found in server";
			}
			else{
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unknown error from server");
				LOG(ERROR)<<"unknown error from server";
			}
			sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
			closeSocket(clientSocketFD);
			return;
		}
	}
	else if(curClient.requestType == TFTP_OPCODE_WRQ){
		LOG(INFO)<<"Write request process initiated";
		std::ofstream fd;
		TftpErrorCode errorCode;
		fd = STARK::getInstance().isFileWritable(curClient.requestFileName, errorCode);
		if(fd.is_open()){
			LOG(INFO)<<"File Open Success";
			bool ret;
			ret = handleReceiveData(curClient, fd);
			if(ret){
				LOG(INFO)<<"All data received";
			}
			else{
				packetSize = 0;
				LOG(ERROR)<<"File not received";
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "error connection terminating");
				sendBufferThroughUDP(sendBuffer, packetSize, curClient.clientSocket, curClient.clientAddress);
			}
			ret = STARK::getInstance().closeWritableFile(curClient.requestFileName, fd);
			if(ret){
				LOG(INFO)<<"File Close Success";
			}
			else{
				LOG(ERROR)<<"File Close Error";
			}
			closeSocket(clientSocketFD);
			return;
		}
		else{
			packetSize = 0;
			if(errorCode == TFTP_ERROR_FILE_ALREADY_EXISTS){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_FILE_ALREADY_EXISTS, "file already exists in server");
				LOG(ERROR)<<"file already exists in server";
			}
			else if(errorCode == TFTP_ERROR_ACCESS_VIOLATION){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ACCESS_VIOLATION, "file access denied in server");
				LOG(ERROR)<<"file access denied in server";
			}
			else{
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unknown error from server");
				LOG(ERROR)<<"unknown error from server";
			}
			sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
			closeSocket(clientSocketFD);
			return;
		}
	}
	else if(curClient.requestType == TFTP_OPCODE_DEL){
		int sendPacketSize;
		LOG(INFO)<<"Delete request process initiated";
		TftpErrorCode errorCode;
		if(STARK::getInstance().isFileDeletable(curClient.requestFileName,errorCode)){
			packetSize = makeACKPacket(sendBuffer,sizeof(sendBuffer),TFTP_VALID_DELETE_ACK);
			sendPacketSize = sendBufferThroughUDP(sendBuffer, packetSize, curClient.defaultServerSocket, curClient.clientAddress);
			if(sendPacketSize!=packetSize){
				LOG(ERROR)<<"Delete valid ACK not sent completely";
				closeSocket(clientSocketFD);
				return;
			}
			LOG(INFO)<<"File: "<<curClient.requestFileName<<", deletion success";
			closeSocket(clientSocketFD);
			return;
		}
		else{
			packetSize = 0;
			if(errorCode == TFTP_ERROR_FILE_NOT_FOUND){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_FILE_NOT_FOUND, "file not found in server");
				LOG(ERROR)<<"file not found in server";
			}
			else if(errorCode == TFTP_ERROR_ACCESS_VIOLATION){
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ACCESS_VIOLATION, "file access denied in server");
				LOG(ERROR)<<"file access denied in server";
			}
			else{
				packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unknown error from server, file not deleted");
				LOG(ERROR)<<"unknown error from server, file not deleted";
			}
			sendPacketSize = sendBufferThroughUDP(sendBuffer, packetSize, clientSocketFD, curClient.clientAddress);
			if(sendPacketSize!=packetSize){
				LOG(ERROR)<<"Error packet not sent completely";	
			}
			closeSocket(clientSocketFD);
			return;
		}
	}
	else{
		LOG(ERROR)<<"invalid opcode";
		packetSize  = 0;
		packetSize = makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "invalid opcode during internal processing");
		sendBufferThroughUDP(sendBuffer, packetSize, clientSocketFD, curClient.clientAddress);
		closeSocket(clientSocketFD);
		return;
	}
	closeSocket(clientSocketFD);
	return;
}

/**
 * @brief function to handle RRQ task for a specific TFTP client.
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
		bool isErrorPktReceived = false;
		while(!allDataSent){
			if(inValidTries > TFTP_RECEIVE_TRIES){
				LOG(ERROR)<<"lost connection";
				return false;
			}
			ret = 0;
			sendPacketSize = 0;
			ackStatus = false;
			isErrorPktReceived = false;
			if(getNewPacket){
				bytesRead = 0;
				bytesRead = readData512(dataBuffer, sizeof(dataBuffer), fd);
				if(bytesRead == -1){
					LOG(ERROR)<<"file read error";
					return false;
				}
				curClient.blockNum++;
			}
			sendPacketSize = makeDataPacket(sendBuffer, sizeof(sendBuffer), curClient.blockNum, dataBuffer, bytesRead);
			if(sendPacketSize == -1){
				LOG(ERROR)<<"unable to make data packet";
				return false;
			}
			LOG(DEBUG)<<"Data packet generated successfully";
			ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, curClient.clientSocket, curClient.clientAddress);
			if(ret != sendPacketSize){
				LOG(ERROR)<<"packet send error";
				return false;
			}
			LOG(DEBUG)<<"Data packet sent successfully";
			ackStatus = getACK(curClient.clientSocket, curClient.clientAddress, curClient.blockNum, isErrorPktReceived);
			if(ackStatus){
				LOG(DEBUG)<<"Valid ACK received";
				inValidTries = 0;
				getNewPacket = true;
				LOG(DEBUG)<<"Bytes read and sent: "<<bytesRead;
				if(bytesRead < TFTP_MAX_DATA_SIZE){
					LOG(DEBUG)<<"All data sent";
					allDataSent = true;
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
		}
		if(allDataSent){
			LOG(INFO)<<"all data sent to client";
			return true;
		}
		return false;
	}
	else{
		LOG(ERROR)<<"file open error";
		return false;
	}
	return false;
}

/**
 * @brief function to handle WRQ task for a specific TFTP client.
*/
bool handleReceiveData(ClientHandler curClient, std::ofstream& fd){
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
			sendPacketSize = makeACKPacket(sendBuffer, sizeof(sendBuffer), curClient.blockNum);
			if(sendPacketSize == -1){
				LOG(ERROR)<<"unable to make data packet";
				return false;
			}
			ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, curClient.clientSocket, curClient.clientAddress);
			if(ret != sendPacketSize){
				LOG(ERROR)<<"packet send error";
				return false;
			}
			LOG(DEBUG)<<"ACK "<<curClient.blockNum<<" sent to client";
	
			dataRecvStatus = getData(curClient.clientSocket,curClient.clientAddress, curClient.blockNum+1, recvData, sizeof(recvData), recvDataLen, isErrorPktReceived);

			if(dataRecvStatus){
				ret = writeData512(recvData, recvDataLen, fd);
				if(ret < 0){
					LOG(ERROR)<<"file write error";
					return false;
				}
				curClient.blockNum++;
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
		}
		if(allDataReceived){
			sendPacketSize = 0;
			sendPacketSize = makeACKPacket(sendBuffer, sizeof(sendBuffer), curClient.blockNum);
			if(sendPacketSize == -1){
				LOG(ERROR)<<"unable to make data packet";
				return false;
			}
			ret = sendBufferThroughUDP(sendBuffer, sendPacketSize, curClient.clientSocket, curClient.clientAddress);
			if(ret != sendPacketSize){
				LOG(ERROR)<<"packet send error";
				return false;
			}
			std::chrono::seconds duration(2);
    		std::this_thread::sleep_for(duration);
			LOG(DEBUG)<<"Sending file block ACK";
			return true;
		}
		else{
			LOG(ERROR)<<"Unexpected error";
			return false;
		}
	}
	else{
		LOG(ERROR)<<"file open error";
		return false;
	}
	return false;
}

/**
 * @brief Function to close socket and wait for 2 seconds.
 * 
 * @param socketFD 
 */
void closeSocket(int socketFD){
	std::chrono::seconds duration(2);
    std::this_thread::sleep_for(duration);
    close(socketFD);
    return;
}


/**
 * @brief Function to List the directory content.
 * 
 */
bool list_dir(std::string dir, std::string&  fname) {

    // Get the current timestamp
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    std::string timestamp = ctime(&now);
    timestamp.pop_back();

    // Create the output filename with timestamp
    std::string output_fname = "output_" + std::to_string(now) + ".txt";
    fname = output_fname;

    // Open the file for writing
    std::ofstream output_file(output_fname);
    // Check if the file is successfully opened
    if (!output_file.is_open()) {
        LOG(ERROR)<<"Error opening output file: "<<output_fname<<std::endl;
        return false;
    }

    // Write the header to the file
    output_file<<"Generated at " <<timestamp<<"."<<std::endl;
    output_file<<"\nList of Files in server :" << std::endl;

    // Iterate through the directory and write the content to the file
    for (const auto & file : std::experimental::filesystem::directory_iterator(dir)) {
        output_file << std::experimental::filesystem::path(file).filename() << std::endl;
    }

    // Close the file
    output_file.close();

    LOG(INFO)<<"Output saved to: "<<output_fname<<std::endl;    
    return true;
}