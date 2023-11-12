#include "huffman.hpp"
//#define DEBUG 1
#define TOSTDOUT 1
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){
    int DEBUG = 0;
    if(argc!=4){
        std::cout<<"Error argc\n";
        exit(1);
    }
    std::string mode(argv[1]);
    std::string fileName(argv[2]);
    DEBUG = std::atoi(argv[3]);
    std::cout<<"Args: "<<mode<<", "<<fileName<<", "<<DEBUG<<"\n";

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
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "logs/logHuff.log");
    
    el::Loggers::reconfigureLogger("default", defaultConf);
    bool ret;
    Huffman Obj(fileName);
    if(mode == "C"){
        ret = Obj.compressFile();
        if(ret)
            LOG(INFO)<<"Return status True";
        else    
            LOG(ERROR)<<"Return status False";
    }
    else if(mode == "D"){
        ret = Obj.decompressFile();
        if(ret)
            LOG(INFO)<<"Return status True";
        else    
            LOG(ERROR)<<"Return status False";
    }
    else{
        LOG(ERROR)<<"Invalide mode";
        return -1;
    }
	return 0;
}