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

    #define COMPRESSION_EXTENSION ".cmp"
class Huffman{
    
    private:
        //Huffman table containing symbol and prefix free code 
        std::map <char,std::string> huffmanTable;
        bool freqCompare(const std::pair<char,long>& a, const std::pair<char,int>& b);
    public:
        std::string textFilePath;
        std::string compressedFilePath;
        Huffman();
        Huffman(std::string textFilePath);
        // Function generates sorted list of symbols based on the frequencies.
        bool generateFrequencyMap(str::vector<std::pair<char,long>>& freqMap);
        bool generateHuffmanTable();
}
#endif