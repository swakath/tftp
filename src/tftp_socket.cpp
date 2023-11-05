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

int createUDPSocket(const char* socketIP, int socketPORT){
    // socket file discriptor to be returned
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // checking if the socket creation succeeded
    if(sockfd == -1){
        LOG(FATAL) << "Error creating socket: "<< strerror(errno);
        exit(EXIT_FAILURE);
    }

	// clearing server address struct
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;  // using IPv4 address family

	if (inet_pton(AF_INET, socketIP, &(serv_addr.sin_addr.s_addr)) !=1) {
        LOG(FATAL) << "Error converting IP address: " << strerror(errno);
        exit(EXIT_FAILURE);
    }

	serv_addr.sin_port = htons(socketPORT); // set network port; if port=0 system determines the port

	// associate the socket with its local address
	int bound = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // checking if the socket bind succeeded
    if(bound == -1){
        LOG(FATAL) << "Error binding socket: "<< strerror(errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

	// retrieve and print server formatted IP address xxx.xxx.xxx.xxx
	char ip[17];
	inet_ntop(serv_addr.sin_family, (void *)&serv_addr.sin_addr, ip, sizeof(ip));
     
	//Debug messages
	sprintf(log_message, "New Socket Created: IP [%s] Port [%d]", ip, ntohs(serv_addr.sin_port));
	LOG(INFO) << log_message;

	return sockfd;
}