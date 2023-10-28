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

void handleIncommingRequests(int defaultServerSock){
	char buffer[1024];
    
	while (!END_SERVER_PROCESS) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        ssize_t bytesReceived = recvfrom(defaultServerSock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddress, &clientAddressLength);

        if (bytesReceived == -1) {
            std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
            break;
        }

        std::cout << "Received " << bytesReceived << " bytes from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " - Data: " << buffer << std::endl;
    }
	return;
}

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

int main(int argc, char* argv[]){
    START_EASYLOGGINGPP(argc, argv);    
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    //defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "logs/logServer.log");
    el::Loggers::reconfigureLogger("default", defaultConf);

    int defaultServerSock;
	defaultServerSock = createUDPSocket(serverIP, DEFAULT_PORT);
    END_SERVER_PROCESS = false;

	std::thread incommingThread(handleIncommingRequests, defaultServerSock);
	std::thread terminationThread(handleServerTermination);

	incommingThread.join();
	terminationThread.join();

	close(defaultServerSock);
	return 0;
}
