/**
 * @file tftp_stark.cpp
 * @brief TFTP Stark.
 *
 * This file contains definations of function for STARK Class
 *
 * @date November 8, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "tftp_stark.hpp"

STARK::STARK(){
	//
}

/**
 * @brief function to set root directory
*/
void STARK::setRootDir(const char* directory){
	if(directory!=NULL)
		root_dir.assign(directory);
	return;
}

/**
 * @brief function to check if file is available in the TFTP root directory
*/

bool STARK::isFileAvailable(std::string fileName){
	if(!fileName.empty()){
		std::string filePath = root_dir + fileName;
		std::ifstream fileRead(filePath.c_str(), std::ios::binary);
		if(fileRead.is_open()){
			LOG(DEBUG)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file available in tftp root directroy";
			fileRead.close();
			return true;
		}else{
			LOG(DEBUG)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file not available in tftp root directroy";
			fileRead.close();
			return false;
		}
	}
	else{
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: empty file name";
		return false;
	}
	return false;
}
/**
 * @brief function to check if a file is having read permission
 * opens the file in read more if the permission exists and return the file object
*/
std::ifstream STARK::isFileReadable(std::string fileName, TftpErrorCode& errorCode){
	errorCode = TFTP_ERROR_ACCESS_VIOLATION;
	if(!fileName.empty()){
		LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: stark processing read for file name:"<<fileName;
		std::string filePath = root_dir + fileName;
		LOG(DEBUG)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name:"<<filePath;
		std::ifstream fd(filePath.c_str(),  std::ios::binary);
		if(fd.is_open()){
			std::lock_guard<std::mutex> lock(mutexObj);
			if (fileData.find(fileName) != fileData.end()){
				if(fileData[fileName].second == false){
					int readerCnt = fileData[fileName].first;
					LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file opened in read mode and reader cnt incremented"<<readerCnt+1;
					fileData[fileName].first = readerCnt + 1; 
					return fd;
				}
				else{
					LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file already opened in write mode. Wait until freed";	
					errorCode = TFTP_ERROR_ACCESS_VIOLATION;
					fd.close();
					return std::ifstream();
				}
			}
			else{
				fileData.insert({fileName, std::make_pair(1,false)});
				LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file opened in read mode and inserted in the map";
				return fd;
			}
		}
		else{
			fd.close();
			LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file not found";
			errorCode = TFTP_ERROR_FILE_NOT_FOUND;
			return std::ifstream();
		}
	}
	else{
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name is NULL";
		return std::ifstream();
	}
	return std::ifstream();
}

/**
 * @brief function to check if a file is having write permission
 * opens the file in write mode if the permission exists and return the file object
*/
std::ofstream STARK::isFileWritable(std::string fileName, TftpErrorCode& errorCode){
	errorCode = TFTP_ERROR_ACCESS_VIOLATION;
	if(!fileName.empty()){
		LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name:"<<fileName;
		std::string filePath = root_dir + fileName;
		LOG(DEBUG)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name:"<<filePath;
		if(this->isFileAvailable(fileName)){
			LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file already exists";
			errorCode = TFTP_ERROR_FILE_ALREADY_EXISTS;
			return std::ofstream();
		}
		else{
			std::ofstream fileWrite(filePath.c_str(), std::ios::binary);
			if(fileWrite.is_open()){
				std::lock_guard<std::mutex> lock(mutexObj);
				if (fileData.find(fileName) == fileData.end()){
					fileData.insert({fileName, std::make_pair(0,true)});
					LOG(INFO)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file opened in write mode and inserted in the map";
					return fileWrite;
				}
				else{
					LOG(FATAL)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file not in directory but present in map.";
					errorCode = TFTP_ERROR_NOT_DEFINED;
					fileWrite.close();
					return std::ofstream();
				}
			}
			else{
				fileWrite.close();
				LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: unable to open file in write mode";
				errorCode = TFTP_ERROR_ACCESS_VIOLATION;
				return std::ofstream();
			}
		}
	}
	else{
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name is NULL";
		return std::ofstream();
	}
	return std::ofstream();
}

/**
 * @brief function to close a readable file (ifstream file)
*/
bool STARK::closeReadableFile(std::string fileName, std::ifstream& fd){
	LOG(DEBUG)<<"fd status:"<<fd.is_open()<<", filename: "<<fileName.empty();

	if(!fileName.empty() && fd.is_open()){
		fd.close();
		std::lock_guard<std::mutex> lock(mutexObj);
		if (fileData.find(fileName) != fileData.end()){
			int readCnt = fileData[fileName].first;
			if(readCnt > 0){
				readCnt--;
				fileData[fileName].first = readCnt;
                LOG(DEBUG)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file closed successfully";
                return true;
			}
			else{
				LOG(FATAL)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file open but read cnt already zero";
				return false;
			}	
		}
		else{
			LOG(FATAL)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: read file open but not in list";
			return false;
		}
	} 
	else{
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name is NULL  or file as already closed name:" <<fileName;
		return false;
	}
    return false;
}


/**
 * @brief function to close a writable file (ofstream file)
*/
bool STARK::closeWritableFile(std::string fileName, std::ofstream& fd){
	if(!fileName.empty() && fd.is_open()){
		fd.close();
		std::lock_guard<std::mutex> lock(mutexObj);
		if (fileData.find(fileName) != fileData.end()){
			bool writeStatus = fileData[fileName].second;
			if(writeStatus == true){
				writeStatus = false;
				fileData[fileName].second = writeStatus;
                LOG(DEBUG)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file closed successfully";
                return true;
			}
			else{
				LOG(FATAL)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: wrte file open but write status already false";
				return false;
			}	
		}
		else{
			LOG(FATAL)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file open but not in list";
			return false;
		}
	} 
	else{
		LOG(ERROR)<<"Function:"<<__FUNCTION__<<", Line:"<<__LINE__<<", msg: file name is NULL  or file as already closed name:" <<fileName;
		return false;
	}
    return true;
}