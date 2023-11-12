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
#define DEBUG 0
#define TOSTDOUT 1
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){
    START_EASYLOGGINGPP(argc, argv);
    if(argc!=2){
        std::cout<<"Invalid number of input arguments. Usage: "<<argv[0]<<" <SERVER_IPv4>";
        return(EXIT_FAILURE);
    }
    std::string serverArgIP(argv[1]);
    std::cout<<"Server IP Set to: "<<serverArgIP<<std::endl;
    const char *homeDir = std::getenv("HOME");   
    std::string rootArgDir(homeDir);
    rootArgDir = rootArgDir + "/tftpServer/";
    std::cout<<"TFTP Directory set to: "<<rootArgDir<<std::endl;

    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime [%level] [%thread] [%func][%line] %msg");
    
    if(DEBUG){
        defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
    }else{
        defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    }
    if(TOSTDOUT){
        defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");
    }else{
        defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    }
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "logs/logServer.log");
    
    el::Loggers::reconfigureLogger("default", defaultConf);

    int defaultServerSock;
	defaultServerSock = createUDPSocket(serverArgIP.c_str(), TFTP_DEFAULT_PORT, TFTP_SERVER_SOCKET_TIMEOUT);
    if(defaultServerSock == -1){
        LOG(FATAL) <<"Unable to open socket in default port "<<TFTP_DEFAULT_PORT;
        exit(EXIT_FAILURE);
    }
    END_SERVER_PROCESS = false;
    STARK::getInstance().setRootDir(rootArgDir.c_str());
	std::thread incommingThread(handleIncommingRequests, defaultServerSock);
	std::thread terminationThread(handleServerTermination);

	incommingThread.join();
	terminationThread.join();

	close(defaultServerSock);
	return 0;
}