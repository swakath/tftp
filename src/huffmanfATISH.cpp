#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
using namespace std;


// to solve store result of huffman encode in  binary format and then make changes in decode function 
// how to store map 

#include <iostream>
#include <fstream>


bool writeToBinaryFile(const char* filename){
    // Your string containing '1' and '0'
    std::string binaryString = "110101010";
    std::ifstream inputFile(filename);
    if(!inputFile.is_open()){
        std::cout <<" Fail to generate binary file due to input file not open" << std::endl;
        return false;
    }
    // Open the binary file for writing
    std::ofstream binaryFile("output.bin", std::ios::binary);

    // Check if the file is open
    if (!binaryFile.is_open()) {
        std::cerr << "Unable to open the file.\n";
        return false;
    }

    // Variable to store packed bits
    char packedBits = 0, c;
    // Variable to keep track of the bit position in the packed byte
    int bitPosition = 0;

    // Iterate through the characters in the string
    while (inputFile.get(c)) {
        // Set the corresponding bit in the packed byte
        packedBits |= (c == '1') ? (1 << bitPosition) : 0;

        // Move to the next bit position
        bitPosition++;

        // If we've filled a byte, write it to the file
        if (bitPosition == 8) {
            binaryFile.write(&packedBits, sizeof(char));

            // Reset the packedBits and bitPosition for the next byte
            packedBits = 0;
            bitPosition = 0;
        }
    }

    // If there are remaining bits, write them to the file
    if (bitPosition > 0) {
        binaryFile.write(&packedBits, sizeof(char));
    }

    // Close the binary file
    binaryFile.close();
    return true;
}

bool readBinaryFile(const char* filename) {
    // Open the binary file for reading
    std::ifstream binaryFile(filename, std::ios::binary);
    if (!binaryFile.is_open()) {
        std::cerr << "Unable to open the file.\n";
        return false;
    }

    // Variable to store unpacked bits
    char unpackedBits;
    // Variable to keep track of the bit position in the unpacked byte
    int bitPosition = 0;

    // Read the file byte by byte
    while (binaryFile.read(&unpackedBits, sizeof(char))) {
        // Iterate through the bits in the byte
        for (int i = 0; i < 8; i++) {
            // Extract the i-th bit from the byte
            char bit = (unpackedBits & (1 << i)) ? '1' : '0';

            // Output the bit (you can modify this to store it in a string, for example)
            std::cout << bit;

            // Move to the next bit position
            bitPosition++;

            // Output a space for better readability
            if (bitPosition % 8 == 0)
                std::cout << ' ';
        }
    }

    // Close the binary file
    binaryFile.close();

    return true;
}

// function for binary addition
string addBinary(string a, string b)
{
    std::string result = "";
    int carry = 0;

    // Make sure both strings have the same length by adding leading zeros if needed
    while (a.length() < b.length())
    {
        a = "0" + a;
    }
    while (b.length() < a.length())
    {
        b = "0" + b;
    }

    for (int i = a.length() - 1; i >= 0; i--)
    {
        int bit1 = a[i] - '0'; // Convert character to integer (0 or 1)
        int bit2 = b[i] - '0'; // Convert character to integer (0 or 1)

        int sum = bit1 + bit2 + carry;
        carry = sum / 2;
        result = to_string(sum % 2) + result;
    }

    if (carry > 0)
    {
        result = "1" + result;
    }

    return result;
}
// function for huffman encode
map<char, string> calcHuffLens(vector<long> &w, vector<char> &z)
{
    int n = w.size();
    int leaf = n - 1;
    int root = n - 1;
    map<char, string> asciiValue;

    for (int next = n - 1; next >= 1; --next)
    {
        // first child
        if (leaf < 0 || (root > next && w[root] < w[leaf]))
        {
            w[next] = w[root];
            w[root] = next;
            root = root - 1;
        }
        else
        {
            w[next] = w[leaf];
            leaf = leaf - 1;
        }
        // second child
        if (leaf < 0 || (root > next && w[root] < w[leaf]))
        {
            w[next] = w[next] + w[root];
            w[root] = next;
            root = root - 1;
        }
        else
        {
            w[next] = w[next] + w[leaf];
            leaf = leaf - 1;
        }
    }
    for (int i = 0; i < w.size(); i++)
    {
        cout << i << " value " << w[i] << endl;
    }
    w[1] = 0;
    for (int next = 2; next <= n - 1; ++next)
    {
        w[next] = w[w[next]] + 1;
    }
    for (int i = 0; i < w.size(); i++)
    {
        cout << i << " value " << w[i] << endl;
    }
    int avail = 1;
    int used = 0;
    int depth = 0;
    root = 1;
    int next = 0;
    while (avail > 0)
    {
        while (root < n && (w[root] == depth))
        {
            used = used + 1;
            root = root + 1;
        }
        while (avail > used)
        {
            w[next] = depth;
            next++;
            avail--;
        }
        avail = 2 * used;
        depth++;
        used = 0;
    }
    for (int i = 0; i < w.size(); i++)
    {
        cout << i << " value " << w[i] << endl;
    }
    // assigning code lexigraphically
    vector<string> prefixCode;
    char zero = '0';
    // setting up length of first character
    string result = "";
    for (int i = 0; i < w[0]; i++)
    {
        result = result + zero;
    }
    prefixCode.push_back(result);

    for (int i = 1; i < w.size(); i++)
    {
        result = "";
        string temp = prefixCode[i - 1];
        // int  previousPrefixSize = stoi(temp);
        temp = addBinary(temp, "1");
        // temp = to_string(previousPrefixSize);
        int tempLength = temp.length();
        for (int j = 0; j < w[i] - tempLength; j++)
        {
            temp = temp + zero;
        }
        result = temp;
        prefixCode.push_back(result);
    }
    for (int i = 0; i < prefixCode.size(); i++)
    {
        cout << i << "i th code " << prefixCode[i] << endl;
    }
    for (int i = 0; i < z.size(); i++)
    {
        asciiValue[z[i]] = prefixCode[i];
    }
    // Iterate over the elements using iterators
    for (auto it = asciiValue.begin(); it != asciiValue.end(); ++it)
    {
        std::cout << it->first << " => " << it->second << std::endl;
    }
    return asciiValue;
}


map<char, string> huffmanencode(string filename){

    unordered_map<char, long> charOccurence;

    ifstream inputFil(filename);

    if (!inputFil.is_open())
    {
        cout << "error opening file " << endl;
        
    }

    char ch;

    while (inputFil.get(ch))
    {
        charOccurence[ch]++;
    }
    // charOcc.sort()

    inputFil.close();

    multimap<long, char> sortedMap;

    for (const auto &pair : charOccurence)
    {
        sortedMap.insert(make_pair(pair.second, pair.first));
    }

    // Print the sorted multimap
    // Traverse the multimap in reverse order
    // vector to store character frequency
    vector<long> charFrequencyInDecreasingOrder;
    //
    vector<char> charInDecreasingOrderOfFrequency;
    for (auto rit = sortedMap.rbegin(); rit != sortedMap.rend(); ++rit)
    {
        std::cout << rit->first << ": " << rit->second << std::endl;
        charFrequencyInDecreasingOrder.push_back(rit->first);
        charInDecreasingOrderOfFrequency.push_back(rit->second);
    }
    for (int i = 0; i < charFrequencyInDecreasingOrder.size(); i++)
    {
        cout << charFrequencyInDecreasingOrder[i] << endl;
    }
    for (int i = 0; i < charFrequencyInDecreasingOrder.size(); i++)
    {
        cout << charInDecreasingOrderOfFrequency[i] << endl;
    }

    map<char, string> resultAsciiValue = calcHuffLens(charFrequencyInDecreasingOrder, charInDecreasingOrderOfFrequency);
    
    ifstream decodeFile(filename);

    if (!decodeFile.is_open())
    {
        cout << "error opening file " << endl;
    }
    char c;


    std::ofstream output("HCoutput.txt", std::ios::out | std::ios::trunc);

    std::ofstream binaryFile("binary_data.bin", std::ios::out | std::ios::binary);

    if (!binaryFile) {
        std::cerr << "Error opening the file for writing." << std::endl;
    }
    // ofstream binaryFil("output.bin", std::ios::binary);
    if (output.is_open())
    {
        ofstream outputStream("exoutput.txt");
        while (decodeFile.get(c))
        {
            
            // binaryFil << resultAsciiValue[c];
            output << resultAsciiValue[c];
            outputStream << c;
        }
        outputStream.close();
        decodeFile.close();
        output.close();
        std::cout << "Data written to "
                  << "HCoutput.txt" << std::endl;
    }
    else
    {
        std::cerr << "Error opening the file for writing." << std::endl;
    }
    writeToBinaryFile("HCoutput.txt");
    readBinaryFile("output.bin");

    return resultAsciiValue;
}
void decodeHuffman(string filename, map<char, string> result){
    // Create a new map with reversed key-value pairs
    std::map<string, char> reverseValue;

    for (const auto &entry : result)
    {
        reverseValue[entry.second] = entry.first;
    }

    string currentCode;
    char c;
    ifstream heoutput(filename);
    ofstream hufmanoutput("HDoutput.txt");
    if (!hufmanoutput.is_open())
    {
        cout << "Error huffman output is not open.";
        return ;
    }
    while(heoutput.get(c))
    {
        currentCode += c;

        // Check if the current code matches a Huffman code
        if (reverseValue.count(currentCode))
        {
            // Append the corresponding character to the decoded data
            hufmanoutput << reverseValue[currentCode];

            // Reset the current code
            currentCode.clear();
        }
    }
    heoutput.close();
    hufmanoutput.close();
}
int main() {
// make this result as global 
 map<char, string> result = huffmanencode("input.txt");
 decodeHuffman("HCoutput.txt",result);
    
}