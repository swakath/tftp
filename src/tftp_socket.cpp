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
int createUDPSocket(const char* socketIP, int socketPORT){
    // socket file discriptor to be returned
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // checking if the socket creation succeeded
    if(sockfd == -1){
        LOG(ERROR) << "Error creating socket: "<< strerror(errno)<<" for port "<<socketPORT;
		return -1;
    }

	// clearing server address struct
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;  // using IPv4 address family

	if (inet_pton(AF_INET, socketIP, &(serv_addr.sin_addr.s_addr)) !=1) {
        LOG(ERROR) << "Error converting IP address: " << strerror(errno)<<" for port "<<socketPORT;;
        return -1;
    }

	serv_addr.sin_port = htons(socketPORT); // set network port; if port=0 system determines the port

	// associate the socket with its local address
	int bound = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // checking if the socket bind succeeded
    if(bound == -1){
        LOG(ERROR) << "Error binding socket: "<< strerror(errno)<<" for port "<<socketPORT;;
        close(sockfd);
        return -1;
    }

	// retrieve and print server formatted IP address xxx.xxx.xxx.xxx
	char ip[17];
	inet_ntop(serv_addr.sin_family, (void *)&serv_addr.sin_addr, ip, sizeof(ip));
     
	//Debug messages
	sprintf(log_message, "New Socket Created: IP [%s] Port [%d]", ip, ntohs(serv_addr.sin_port));
	LOG(INFO) << log_message;

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
		return -1;
	}
}

/**
 * @brief 
*/
int sendBufferThroughUDP(uint8_t* sendBuffer, size_t bufferLen, int socketfd, struct sockaddr_in clientAddress){
	if(sendBuffer!=NULL && bufferLen > 0 && socketfd > 0){
		size_t sendLen;
		socklen_t clientAddressLength = sizeof(clientAddress);
		sendLen = sendto(socketfd, sendBuffer, bufferLen, 0, (struct sockaddr*)&clientAddress, clientAddressLength);
		if(sendLen == -1){
			LOG(ERROR)<<"Function: "<<__FUNCTION__<<" Line:"<<__LINE__<<" msg: Data not send";
			return -1;
		}
		else if(sendLen != bufferLen){
			LOG(ERROR)<<"Function: "<<__FUNCTION__<<" Line:"<<__LINE__<<" msg: Full buffer not sent";
			return -1;
		}
		else{
			LOG(INFO)<<"Function: "<<__FUNCTION__<<" Line:"<<__LINE__<<" msg: Packet sent successfully";
			return sendLen;
		}
	}
	return -1;
}