/**
 * @file tftp_stark.hpp
 * @brief TFTP Start.
 *
 * @brief "STARK" stands for "Simple Tracker for Accessing and Recording for file Keeping" 
 * Singleton class to manage all the file access previlages
 *
 * @date November 8, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/

#ifndef TFTP_STARK_H
#define TFTP_STARK_H

#ifndef COMM_H
    #include "tftp_common.hpp"
#endif

#ifndef SINGLETON_H
    #include "singleton.hpp"
#endif

class STARK : public Singleton<STARK> {
    friend class Singleton<STARK>;
    protected:
        STARK();
        std::mutex mutexObj;
    public:
        std::string root_dir;
        std::unordered_map<std::string, std::pair<int, bool>> fileData;
        void setRootDir(const char* directory);
        bool isFileAvailable(std::string fileName);
        bool isFileDeletable(std::string fileName, TftpErrorCode& errorCode);
        std::ifstream isFileReadable(std::string fileName, TftpErrorCode& errorCode);
        std::ofstream isFileWritable(std::string fileName, TftpErrorCode& errorCode);
        bool closeReadableFile(std::string fileName, std::ifstream& fd);
        bool closeWritableFile(std::string fileName, std::ofstream& fd);
        
};

#endif