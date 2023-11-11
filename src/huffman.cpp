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

#include "huffman.h"

/**
 * @brief Construct a new Huffman:: Huffman object
 * 
 */
Huffman::Huffman(){
    this->textFilePath = "";
    this->compressedFilePath= "";
}

/**
 * @brief Construct a new Huffman:: Huffman object
 * 
 * @param filePath 
 */
Huffman::Huffman(std::string filePath){
    this->textFilePath = filePath;
    this->compressedFilePath = filepath + COMPRESSION_EXTENSION;
}

/**
 * @brief Function to compare two <char,int> pair
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false 
 */
bool Huffman::freqCompare(const std::pair<char,int>& a, const std::pair<char,int>& b){
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
bool Huffman::generateFrequencyMap(str::vector<std::pair<char,long>>& freqMap){
    std::unordered_map<char, long> symbolFreqs;
    std::ifstream inputFile(this->textFilePath.c_str());
    if(inputFile.is_open()){
        char symbol;
        while(inputFile.get(symbol)){
            symbolFreqs[symbol]++;
        }
        inputFile.close();
        for(const auto& curSymbol : symbolFreqs){
            freqMap.insert(std::make_pair(curSymbol.first, curSymbol.second));
        }
        delete(symbolFreqs);
        std::sort(freqMap.begin(), freqMap.end(), this->freqCompare);
        return true;
    }else{
        LOG(ERROR)<<"File open error";
        return false;
    }
    LOG(ERROR)<<"Open condidtion";
    return false;
}

bool Huffman::generateHuffmanTable(){
    str::vector<std::pair<char,long>> freqMap;
    bool ret;
    ret = generateFrequencyMap(freqMap);
    if(ret){
        int totalSymbols = freqMap.size();
        std::vector<long> freqList(0,totalSymbols);
        for(const auto& curSymbol : symbolFreqs){
            freqList.push_back(curSymbol.second);
        }
    }else{
        LOG(ERROR)<<"Error generating frequency map";
        return false;
    }
    LOG(ERROR)<<"Open condition";
    return false;
}



