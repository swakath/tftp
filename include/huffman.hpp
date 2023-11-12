/**
 * @file huffman.cpp
 * @brief Huffman class prtotype  to generate huffman codes
 * from a given text file.
 *
 * @date November 07, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 
#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <utility>
#include <easylogging++.h>
#include <bitset>

#define COMPRESSION_EXTENSION ".cmp"
#define END_TO_TRANSMISSION 0x04
class Huffman {
    private:
        //Huffman table containing symbol and prefix free code 
        std::map<char,std::string> encodeHuffmanTable;
        std::map<std::string, char> decodeHuffmanTable;
        std::string addBinary(const std::string& a, const std::string& b);
        std::vector<std::pair<char,uint8_t>> headerInfo;
    public:
        std::string textFilePath;
        std::string compressedFilePath;
        Huffman();
        Huffman(std::string textFilePath);
        // Function generates sorted list of symbols based on the frequencies.
        bool generateFrequencyMap(std::vector<std::pair<char,long>>& freqMap);
        bool generateHuffmanTableFromFile();
        bool compressFile();
        bool decompressFile();
        bool writeHeader(std::ofstream& fd);
        bool readHeader(std::ifstream& fd);
        bool generateTablesFromHeader();
};

bool freqCompare(const std::pair<char,long>& a, const std::pair<char,long>& b);
#endif