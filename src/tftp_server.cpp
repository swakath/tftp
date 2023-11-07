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
	memset(requestFileName, 0, sizeof(requestFileName));

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
	strcpy(this->requestFileName, requestFileName);
	// Currently only OCTET mode is supported
	strcpy(this->operationMode , operationMode);
}

/**
 * @brief debug print funtion for ClientHander Class
*/
void ClientHandler::printVals(){
	memset(log_message,0,sizeof(log_message));
	sprintf(log_message, "Client Obj Vals: IP[%s] Port[%d] fileName[%s] mode[%s]", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), requestFileName, operationMode);
	LOG(INFO) << log_message;
	return;
}

/**
 * @brief function to handle incomming client request to default TFTP server port
*/
void handleIncommingRequests(int serverSock){
	
	char recvBuffer[TFTP_MAX_PACKET_SIZE];
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];
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
            LOG(ERROR) << "Error receiving data: " << strerror(errno);
            continue;
        }

		LOG(INFO) << "Received " << bytesReceived << " bytes from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " - Data:" << recvBuffer ;
		
		// Checking sanity of recvBuffer
		if(sizeof(recvBuffer) <= TFTP_MIN_CONN_INIT_PACKET_SIZE*sizeof(uint8_t)){
			LOG(ERROR) <<"Invalid request in port: "<<TFTP_DEFAULT_PORT;
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
			LOG(ERROR)<<"Recv"<<opcode<<", Comp"<<TFTP_OPCODE_RRQ<<":"<<TFTP_OPCODE_WRQ;
			LOG(ERROR)<<"Incompatable OPCODE received from "<< inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port);
			makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ILLEGAL_OPERATION, "opcode invalid");
			sendBufferThroughUDP(sendBuffer, sizeof(sendBuffer), serverSock, clientAddress);
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
			LOG(ERROR)<<"Offset value" << 2+strlen(fileName)+1;
			LOG(ERROR)<<"Incompatable MODE received from "<< inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port)<<"mode: "<<mode;
			makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_ILLEGAL_OPERATION, "incompatable mode");
			sendBufferThroughUDP(sendBuffer, sizeof(sendBuffer), serverSock, clientAddress);
			continue;
		}

		memset(log_message,0,sizeof(log_message));
		sprintf(log_message, "Connection request received: IP[%s] Port[%d] fileName[%s] mode[%s]", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fileName, mode);
		LOG(INFO) << log_message;

		ClientHandler curClientHandlerObj(serverSock ,clientAddress, opcode, fileName, mode);
		clientObjects.emplace_back(curClientHandlerObj);
		curClientHandlerObj.printVals();
		//std::thread curClientHandle(handleClient, curClientHandlerObj);
		//clientThreads.emplace_back(curClientHandle);
	}

	/*
	for(int i = ; i<clientThreads.size(); ++i){
		clientThreads[i].join();
	}
	*/

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
	int clientPort = 0;
	int clientSocketFD = 0;
	clientSocketFD = createRandomUDPSocket(serverIP, &clientPort);
	uint8_t sendBuffer[TFTP_MAX_PACKET_SIZE];

	if(clientSocketFD == -1){
		LOG(ERROR)<<"Unable to create UPD Socket for client: IP"<<inet_ntoa(curClient.clientAddress.sin_addr) << ": PORT" << ntohs(curClient.clientAddress.sin_port);
		makeErrorPacket(sendBuffer,sizeof(sendBuffer), TFTP_ERROR_NOT_DEFINED, "unable to create new socket");
		sendBufferThroughUDP(sendBuffer, sizeof(sendBuffer), curClient.defaultServerSocket, curClient.clientAddress);
		return;
	}
	if(curClient.requestType == TFTP_OPCODE_RRQ){

	}

}
