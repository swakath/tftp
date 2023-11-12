/**
 * @file huffman.cpp
 * @brief Huffman class defination to generate huffman codes
 * from a given text file.
 *
 * @date November 07, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 

#include "huffman.hpp"

/**
 * @brief Construct a new Huffman:: Huffman object
 * 
 */
Huffman::Huffman(){
    this->textFilePath = "";
    this->compressedFilePath= "";
    this->textFileName = "";
    this ->compressFileName= "";
}

/**
 * @brief Construct a new Huffman:: Huffman object
 * 
 * @param filePath 
 */
Huffman::Huffman(std::string filePath){
    this->textFilePath = filePath;
    this->compressedFilePath = filePath + COMPRESSION_EXTENSION;
}

/**
 * @brief Function to set the text file path
 * 
 * @param filePath 
 */
void Huffman::setFileName(std::string fileName){
    this->textFileName = fileName;
    this->compressFileName = fileName + COMPRESSION_EXTENSION;
    this->textFilePath = this->root_dir + this->textFileName ;
    this->compressedFilePath =  this->root_dir + this->compressFileName;
}

/**
 * @brief Function to set the directory
 * 
 */
void Huffman::setRootDir(std::string directory){
    this->root_dir = directory;
}

/**
 * @brief Function to compare two <char,int> pair
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false 
 */
bool freqCompare(const std::pair<char,long>& a, const std::pair<char,long>& b){
    if(a.second > b.second){
        return true;
    }
    return false;
}

/**
 * @brief Fucntion to generate frequecy table for symbols using input file.
 * The generated map is in decreasing order of frequencies
 * 
 * @param freqMap 
 * @return true 
 * @return false 
 */
bool Huffman::generateFrequencyMap(std::vector<std::pair<char,long>>& freqMap){
    std::unordered_map<char, long> symbolFreqs;
    std::ifstream inputFile(this->textFilePath.c_str());
    if(inputFile.is_open()){
        char symbol;
        while(inputFile.get(symbol)){
            symbolFreqs[symbol]++;
        }
        inputFile.close();
        for(const auto& curSymbol : symbolFreqs){
            freqMap.push_back(std::make_pair(curSymbol.first, curSymbol.second));
        }
        char EOT = END_TO_TRANSMISSION;
        freqMap.push_back(std::make_pair(EOT,1));
        std::sort(freqMap.begin(), freqMap.end(), freqCompare);
        return true;
    }else{
        inputFile.close();
        LOG(ERROR)<<"File open error";
        return false;
    }
    LOG(ERROR)<<"Open condidtion";
    return false;
}


std::string Huffman::addBinary(const std::string& a, const std::string& b) {
    std::string result = "";
    int carry = 0;
    int maxLength = std::max(a.length(), b.length());
    std::string paddedA = std::string(maxLength - a.length(), '0') + a;
    std::string paddedB = std::string(maxLength - b.length(), '0') + b;

    for (int i = maxLength - 1; i >= 0; --i) {
        int bit1 = paddedA[i] - '0';
        int bit2 = paddedB[i] - '0';
        int sum = bit1 + bit2 + carry;
        carry = sum / 2;
        result = std::to_string(sum % 2) + result;
    }

    if (carry > 0) {
        result = "1" + result;
    }

    return result;
}

bool Huffman::generateHuffmanTableFromFile(){
    std::vector<std::pair<char,long>> freqMap;
    bool ret;
    ret = generateFrequencyMap(freqMap);
    if(ret){
        int totalSymbols = freqMap.size();
        std::vector<long> freqList(0,totalSymbols);

        for(const auto& curSymbol : freqMap){
            freqList.push_back(curSymbol.second);
        }
        int curLeaf = totalSymbols - 1;
        int curRoot = totalSymbols - 1;

        for (int cur = totalSymbols - 1; cur >= 1; --cur)
        {
            if (curLeaf < 0 || curRoot > cur && freqList[curRoot] < freqList[curLeaf])
            {
                freqList[cur] = freqList[curRoot];
                freqList[curRoot] = cur;
                curRoot = curRoot - 1;
            }
            else
            {
                freqList[cur] = freqList[curLeaf];
                curLeaf = curLeaf - 1;
            }

            if (curLeaf < 0 || curRoot > cur && freqList[curRoot] < freqList[curLeaf])
            {
                freqList[cur] = freqList[cur] + freqList[curRoot];
                freqList[curRoot] = cur;
                curRoot = curRoot - 1;
            }
            else
            {
                freqList[cur] = freqList[cur] + freqList[curLeaf];
                curLeaf = curLeaf - 1;
            }
        }
        
        for(int i = 0; i<freqList.size();++i){
            LOG(DEBUG)<<freqMap[i].first<<", freq: "<<freqList[i]<<" Size of freq"<<freqList.size();
        }

        LOG(DEBUG)<<"Working here";
        freqList[1] = 0;
        for (int cur = 2; cur <= totalSymbols - 1; ++cur)
        {
            LOG(DEBUG)<<freqList[cur];
            freqList[cur] = freqList[freqList[cur]] + 1;
        }

        for(int i = 0; i<freqList.size();++i){
            LOG(DEBUG)<<freqMap[i].first<<", freq: "<<freqList[i];
        }

        int available = 1;
        int used = 0;
        int depth = 0;
        curRoot = 1;
        int cur = 0;
        while (available > 0)
        {
            while (curRoot < totalSymbols && (freqList[curRoot] == depth))
            {
                used = used + 1;
                curRoot = curRoot + 1;
            }
            while (available > used)
            {
                freqList[cur] = depth;
                cur++;
                available--;
            }
            available = 2 * used;
            depth++;
            used = 0;
        }
        
        for(int i = 0; i<freqList.size();++i){
            LOG(DEBUG)<<freqMap[i].first<<", freq: "<<freqList[i];
        }
        
        // Huffman header info
        for(int i = 0; i<freqList.size();++i){
            uint8_t temp = freqList[i] & 0xFF;

            this->headerInfo.push_back(std::make_pair(freqMap[i].first, temp));
        }

        for(int i = 0; i<this->headerInfo.size(); ++i){
            LOG(INFO)<<"Header:" << this->headerInfo[i].first<<", freq: "<<(int)this->headerInfo[i].second;
        }

        bool ret;
        ret = generateTablesFromHeader();
        if(!ret){
            LOG(ERROR)<<"Error generating tables from headers";
            return false;
        }
        return true;

    }else{
        LOG(ERROR)<<"Error generating frequency map";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}

/**
 * @brief Function to compress a given text file.
 * 
 * @return true 
 * @return false 
 */
bool Huffman::compressFile(){
    if(!this->textFilePath.empty() && !this->compressedFilePath.empty()){
        bool ret;
        ret = generateHuffmanTableFromFile();
        if(!ret){
            LOG(ERROR)<<"Error generating huffman table";
            return false;
        }

        std::ifstream inFile(this->textFilePath.c_str(), std::ios::in);
        if(!inFile){
            LOG(ERROR)<<"Error opening file: "<<this->textFilePath;
            return false;
        }

        std::ofstream outFile(this->compressedFilePath.c_str(), std::ios::out | std::ios::binary);
        if(!outFile){
            LOG(ERROR)<<"Error opening file: "<<this->compressedFilePath;
            return false;
        }

        ret = writeHeader(outFile);
        if(!ret){
            LOG(ERROR)<<"Header write error";
            return false;
        }
        LOG(DEBUG)<<"Header write success";

        char symbol;
        std::string strBuffer;
        while(inFile.get(symbol)){
            if(this->encodeHuffmanTable.find(symbol)!=this->encodeHuffmanTable.end()){
                strBuffer += encodeHuffmanTable[symbol];
                while(strBuffer.size()>=8){
                    std::bitset<8> byte(strBuffer.substr(0,8));
                    char encodedByte = static_cast<char>(byte.to_ulong());
                    outFile.put(encodedByte);
                    if(!outFile){
                        LOG(ERROR)<<"File write error";
                        inFile.close();
                        outFile.close();
                        return false;
                    }
                    LOG(DEBUG)<<"Put val"<<strBuffer.substr(0,8);
                    strBuffer.erase(0,8);
                }
            }
            else{
                LOG(ERROR)<<"Symbol not found in the huffman table";
                return false;
            }
        }
        char EOT = END_TO_TRANSMISSION;
        if(this->encodeHuffmanTable.find(EOT) == this->encodeHuffmanTable.end()){
            LOG(ERROR)<<"EOT Symbol not found in the huffman table";
            inFile.close();
            outFile.close();
            return false;
        }
        strBuffer += this->encodeHuffmanTable[EOT];

        // Pad the buffer to ensure it's a multiple of 8
        while (strBuffer.size() % 8 != 0) {
            strBuffer += '0';
        }
        LOG(INFO)<<"Last buffer:"<<strBuffer<<",EOT: "<<this->encodeHuffmanTable[EOT];
        while(strBuffer.size()>=8){
            std::bitset<8> byte(strBuffer.substr(0,8));
            char encodedByte = static_cast<char>(byte.to_ulong() & 0xFF);
            outFile.put(encodedByte);
            if(!outFile){
                LOG(ERROR)<<"File write error";
                inFile.close();
                outFile.close();
                return false;
            }
            LOG(DEBUG)<<"Put val"<<strBuffer.substr(0,8);
            strBuffer.erase(0,8);
        }

        if(strBuffer.size()>0){
            LOG(ERROR)<<"Buffer not written completely";
            inFile.close();
            outFile.close();
            return false;
        }
        inFile.close();
        outFile.close();
        return true;
    }else{
        LOG(ERROR)<<"Invalid file paths";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}

bool Huffman::decompressFile(){
    if(!this->textFilePath.empty() && !this->compressedFilePath.empty()){
        bool ret;
        std::ifstream inFile(this->compressedFilePath.c_str(), std::ios::in | std::ios::binary);
        if(!inFile){
            LOG(ERROR)<<"Error opening file: "<<this->compressedFilePath;
            return false;
        }

        ret = readHeader(inFile);
        if(!ret){
            LOG(ERROR)<<"Error in reading the header";
            return false;
        }

        ret = generateTablesFromHeader();
        if(!ret){
            LOG(ERROR)<<"Error in generating table from header";
            return false;
        }
        //std::string otherOut = "D" + this->textFilePath;
        //std::ofstream outFile(otherOut.c_str(), std::ios::out);
        std::ofstream outFile(this->textFilePath.c_str(), std::ios::out);
        if(!outFile){
            LOG(ERROR)<<"Error opening file: "<<this->textFilePath;
            return false;
        }
        std::string buffer;
        char byte;
        bool isEOTReceived = false;
        char EOT = END_TO_TRANSMISSION;

        while(inFile.get(byte) && !isEOTReceived){
            std::bitset<8> bits(byte);
            buffer += bits.to_string();
            bool cntRead = true;
            while(cntRead){
                for (const auto& entry : this->decodeHuffmanTable) {
                    while (buffer.size() >= entry.first.size()) {
                        if (buffer.substr(0, entry.first.size()) == entry.first) {
                            if(!outFile){
                                LOG(ERROR)<<"Error writing to file";
                                inFile.close();
                                outFile.close();
                                return false;
                            }
                            buffer.erase(0, entry.first.size());
                            if(entry.first == this->encodeHuffmanTable[EOT]){
                                isEOTReceived = true;
                            }else{
                                outFile.put(entry.second);
                            }
                        } else {
                            break;
                        }
                    }
                    if(buffer.size() < entry.first.size() || isEOTReceived){
                        cntRead = false;
                        break;
                    }
                }
                if(buffer.size() < this->decodeHuffmanTable.begin()->first.size() || (buffer.size() == this->decodeHuffmanTable.begin()->first.size() && buffer!=this->decodeHuffmanTable.begin()->first)){
                    cntRead = false;
                }
            }
        }
        if(!isEOTReceived){
            LOG(ERROR)<<"Did not receive the End to termination";
            inFile.close();
            outFile.close();
            return false;
        }

        inFile.close();
        outFile.close();
        return true;
    }else{
        LOG(ERROR)<<"Invalid file paths";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}

bool Huffman::writeHeader(std::ofstream& fd){
    if(fd.is_open()){
        for(int i = 0; i< this->headerInfo.size(); ++i){
            fd.put(this->headerInfo[i].first);
            if(!fd){
                LOG(ERROR)<<"File write error";
                return false;
            }
            fd.put(this->headerInfo[i].second);
            if(!fd){
                LOG(ERROR)<<"File write error";
                return false;
            }
        }
        LOG(DEBUG)<<"Header write success";
        return true;
    }else{
        LOG(ERROR)<<"File open error";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}

bool Huffman::readHeader(std::ifstream& fd){
    if(fd.is_open()){
        bool isEOTReceived = false;
        this->headerInfo.clear();
        while(!isEOTReceived){
            char symbol;
            char freq;
            fd.get(symbol);
            if(!fd){
                LOG(ERROR)<<"File reading error";
                return true;
            }
            fd.get(freq);
            if(!fd){
                LOG(ERROR)<<"File reading error";
                return true;
            }
            LOG(DEBUG)<<"symbol: "<<symbol<<"freq: "<<(uint8_t)freq;
            this->headerInfo.push_back(std::make_pair(symbol, (uint8_t)freq));
            if(symbol == END_TO_TRANSMISSION){
                isEOTReceived = true;
            }
        }

        for(int i = 0; i< this->headerInfo.size(); i++){
            int temp = (uint8_t)this->headerInfo[i].second;
            LOG(DEBUG)<<"Header, "<<this->headerInfo[i].first<<", freq:"<<temp;
        }
        LOG(DEBUG)<<"Header read from file";
        return true;
    }else{
        LOG(ERROR)<<"File open error";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}

bool Huffman::generateTablesFromHeader(){
    if(this->headerInfo.size()>0){
        std::vector<std::string> huffmanCodes;
        std::string curCode = "";
        int curCodeLen = 0;
        int readSize = (int)this->headerInfo[0].second;
        curCode = std::string(readSize, '0');
        huffmanCodes.push_back(curCode);
        
        for (int i = 1; i < this->headerInfo.size(); ++i) {
            curCode = "";
            curCode = addBinary(huffmanCodes[i - 1], "1");
            curCodeLen = curCode.length();
            readSize = (int)this->headerInfo[i].second;
            curCode = curCode + std::string(readSize - curCodeLen, '0');
            huffmanCodes.push_back(curCode);
        }

        this->encodeHuffmanTable.clear();
        for(int i = 0;  i < this->headerInfo.size(); ++i){
            this->encodeHuffmanTable[this->headerInfo[i].first] = huffmanCodes[i];
        }
        this->decodeHuffmanTable.clear();

        for(int i = 0;  i < this->headerInfo.size(); ++i){
            this->decodeHuffmanTable[huffmanCodes[i]] = this->headerInfo[i].first;
        }
        LOG(DEBUG)<<"Table made";
        return true;
    }
    else{
        LOG(ERROR)<<"No header data";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}