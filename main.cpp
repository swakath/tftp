#include "include/common.hpp"
#include "include/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]){

    START_EASYLOGGINGPP(argc, argv);
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "logs/logfile.log");
    el::Loggers::reconfigureLogger("default", defaultConf);

    LOG(INFO) << "Ankit";
    return 0;
}