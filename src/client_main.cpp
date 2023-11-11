/**
 * @file client_main.cpp
 * @brief TFTP Client main function.
 *
 * Main function for tftp client, implementation as per RFC 1350
 *
 * @date November 10, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/

#include "tftp_client.hpp"
#define DEBUG 1
#define TOSTDOUT 1
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){

    if(argc!=4){
        std::cout<<"Invalid number of input arguments. Usage: "<<argv[0]<<" <TFTP_OPERATION> <FILE_NAME> <SERVER_IP>";
        return(EXIT_FAILURE);
    }

    std::string tftpMode(argv[1]);
    std::string requestFileName(argv[2]);
    std::string serverIP(argv[3]);

    if(tftpMode!=CLIENT_READ && tftpMode!=CLIENT_WRITE && tftpMode!=CLIENT_DELETE){
        std::cout<<"Invalid mode. Usage: <TFTP_OPERATION> = READ|WRITE";
        return(EXIT_FAILURE);
    }
    

    START_EASYLOGGINGPP(argc, argv);    
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
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "logs/logClient.log");
    el::Loggers::reconfigureLogger("default", defaultConf);

	bool ret;
    std::string rootDir(clientDir);
    TftpOpcode requestType;

    if(tftpMode == CLIENT_READ){
        requestType = TFTP_OPCODE_RRQ;
        LOG(INFO)<<"RRQ request set";
    }
    else if(tftpMode == CLIENT_WRITE){
        requestType = TFTP_OPCODE_WRQ;
        LOG(INFO)<<"WRQ request set";
    }
    else if(tftpMode == CLIENT_DELETE){
        requestType = TFTP_OPCODE_DEL;
        LOG(INFO)<<"DEL request set";  
    }
    else{
        LOG(ERROR)<<"Invalide tftpMode";
        exit(EXIT_FAILURE);
    }
    STARK::getInstance().setRootDir(clientDir);
    ret = clientManager::getInstance().commInit(rootDir,requestFileName, serverIP, requestType);
	
    if(!ret){
        LOG(FATAL) <<"Unable to open socket in default port "<<TFTP_DEFAULT_PORT;
        exit(EXIT_FAILURE);
    }

    clientManager::getInstance().handleTFTPConnection();
    clientManager::getInstance().commExit();
    return 0;
}