#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdarg>
#include <cstdint>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <mutex>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
using namespace std;

int readData512(uint8_t* dataBuffer, size_t bufferLen, std::ifstream& fd){
    if(dataBuffer!=NULL && bufferLen >= 512 && fd.is_open()){
        cout<<"I am in"<<endl;
        memset(dataBuffer, 0, bufferLen);
        int bytesRead;
        if(!fd.eof()){
            fd.read(reinterpret_cast<char*>(dataBuffer), bufferLen);
            bytesRead = static_cast<int>(fd.gcount());
            if(fd.bad()){
                cout<<__LINE__<<endl;
                return bytesRead;
            }
            
            cout<<__LINE__<<" Read "<<bytesRead<<endl;
            return bytesRead;
        }
        else{
           cout<<__LINE__<<" Read 0"<<endl;
           return 0;
        }
    }
    else{
        cout<<__LINE__<<endl;
        return -1;
    }
    return -1;
}

int main(){
    string fileName = "sample.txt";
    std::ifstream fileRead(fileName.c_str(), std::ios::binary);
    if(!fileRead){
        cout<<"Error"<<__LINE__<<endl;
    }
    cout<<"Success in creating fd"<<endl;

    uint8_t buffer[512];
    int ret;
    ret = readData512(buffer, sizeof(buffer), fileRead);
    if(ret > 0){
        for(int i = 0; i<ret; ++i){
            printf("%c", buffer[i]);
        }
    }else{
        cout<<"Shit";
    }
    cout<<endl;
}