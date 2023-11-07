/**
 * @file server_main.cpp
 * @brief TFTP Socket.
 *
 * Main function for tftp server, implementation as per RFC 1350
 *
 * @date November 7, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/

#include "tftp_server.hpp"
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){
    START_EASYLOGGINGPP(argc, argv);    
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    //defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "logs/logServer.log");
    el::Loggers::reconfigureLogger("default", defaultConf);

    int defaultServerSock;
	defaultServerSock = createUDPSocket(serverIP, TFTP_DEFAULT_PORT);
    END_SERVER_PROCESS = false;

	std::thread incommingThread(handleIncommingRequests, defaultServerSock);
	std::thread terminationThread(handleServerTermination);

	incommingThread.join();
	terminationThread.join();

	close(defaultServerSock);
	return 0;
}