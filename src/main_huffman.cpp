#include "huffman.hpp"
#define DEBUG 0
#define TOSTDOUT 1
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){
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

    std::string fileName = "testFile.txt";

    Huffman Obj(fileName);
    //Obj.compressFile();
    Obj.decompressFile();

	return 0;
}