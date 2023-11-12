/**
 * @file packetMaking.cpp
 * @brief Unit testing for various operations involved in packet creation and respective error handling. 
 *
 * @date November 11, 2023
 * @author Manish Kumar
 * Contact mkubn04@gmail.com
 * 
 * MIT License
*/ 
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include "tftp_packets.hpp"
#include "tftp_stark.hpp"

class TFTPTest : public testing::Test {};

class TftpPosErrorPacketTesting : public testing::TestWithParam<std::tuple<TftpErrorCode, const char*>> {
};

TEST_P(TftpPosErrorPacketTesting, ErrorPacket) {
    TftpErrorCode errorCode = std::get<0>(GetParam());
    const char* errMsg = std::get<1>(GetParam());

    uint8_t errorPacket[TFTP_MAX_PACKET_SIZE];
    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    int ret;

    // Generating the expected error packet
    int indx = 0;
    uint16_t temp;

    // OPCODE
    temp = htons(TFTP_OPCODE_ERROR);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx + 1] = (uint8_t)(temp >> 8 & 0xFF);
    indx += 2;

    // ERROR_CODE
    temp = htons(errorCode);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx + 1] = (uint8_t)(temp >> 8 & 0xFF);
    indx += 2;

    // ERROR_MSG
    memcpy(expected + indx, errMsg, strlen(errMsg));
    indx += strlen(errMsg);

    // NULL_TERMINATION
    expected[indx] = 0x00;

    ret = makeErrorPacket(errorPacket, sizeof(errorPacket), errorCode, errMsg);

    // Assertions
    ASSERT_GT(ret, 0);
    ASSERT_EQ(memcmp(expected, errorPacket, ret), 0);
}

std::vector<std::tuple<TftpErrorCode, const char*>> errorTestCases = {
    std::make_tuple(TFTP_ERROR_NOT_DEFINED, "error packet 0"),
    std::make_tuple(TFTP_ERROR_FILE_NOT_FOUND, "error packet 1"),
    std::make_tuple(TFTP_ERROR_ACCESS_VIOLATION, "error packet 2"),
    std::make_tuple(TFTP_ERROR_DISK_FULL, "error packet 3"),
    std::make_tuple(TFTP_ERROR_ILLEGAL_OPERATION, "error packet 4"),
    std::make_tuple(TFTP_ERROR_UNKNOWN_TID, "error packet 5"),
    std::make_tuple(TFTP_ERROR_FILE_ALREADY_EXISTS, "error packet 6"),
    std::make_tuple(TFTP_ERROR_NO_SUCH_USER, "error packet 7")
};

INSTANTIATE_TEST_SUITE_P(ErrorPackets, TftpPosErrorPacketTesting, testing::ValuesIn(errorTestCases));


// Fail
TEST(TFTP_NEG_ERROR_PACKET_TESTING, ErrorPacket8){
    TftpErrorCode errorCode = TFTP_ERROR_FILE_NOT_FOUND;

    uint8_t errorPacket[TFTP_MAX_PACKET_SIZE];
    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    const char* errMsg = "error packet 8";
    int ret;
    
    // Generating the expected error packet
    int indx = 0;
    uint16_t temp;

    // OPCODE
    temp = htons(TFTP_OPCODE_ERROR);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx+1] = (uint8_t)(temp>>8 & 0xFF);
    indx += 2;

    // Skipped ERROR_CODE
    
    // ERROR_MSG
    memcpy(expected+indx,  errMsg, strlen(errMsg));
    indx += strlen(errMsg);

    // NULL_TERMINATION
    expected[indx] = 0x00;
    
    // function call
    ret = makeErrorPacket(errorPacket, sizeof(errorPacket), errorCode, errMsg);

    // Assertions 
    ASSERT_GT(ret, 0);
    ASSERT_NE(memcmp(expected, errorPacket, ret), 0);
}

// Fail
TEST(TFTP_NEG_ERROR_PACKET_TESTING, ErrorPacket9){
    TftpErrorCode errorCode = TFTP_ERROR_ACCESS_VIOLATION;

    uint8_t errorPacket[TFTP_MAX_PACKET_SIZE];
    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    const char* errMsg = "error packet 9";
    int ret;
    
    // Generating the expected error packet
    int indx = 0;
    uint16_t temp;
    
    // OPCODE
    temp = htons(TFTP_OPCODE_ERROR);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx+1] = (uint8_t)(temp>>8 & 0xFF);
    indx += 2;

    // ERROR_CODE
    temp = htons(errorCode);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx+1] = (uint8_t)(temp>>8 & 0xFF);
    indx += 2;

    // Skipped ERROR_MSG
    
    // NULL_TERMINATION
    expected[indx] = 0x00;
    
    // function call
    ret = makeErrorPacket(errorPacket, sizeof(errorPacket), errorCode, errMsg);
    
    // Assertions
    ASSERT_GT(ret, 0);
    ASSERT_NE(memcmp(expected, errorPacket, ret), 0);
}

// CommInitPacket - comm. initialize packet
TEST(TFTP_INIT_PACKET_TESTING, MakeComInitPacketTest) {
    const size_t bufferLen = 1024;  
    const char* fileName = "testfile.txt";  
    const char* mode = "octet";  
    uint8_t sendBuffer[bufferLen];
    TftpOpcode opcode = TFTP_OPCODE_RRQ;  

    // function call
    int result = makeComInitPacket(opcode, sendBuffer, bufferLen, fileName, mode);

    // Assertions
    EXPECT_GT(result, 0);   
    EXPECT_LE(result, static_cast<int>(bufferLen)); 
}

// Data Packet 
TEST(TFTP_DATA_PACKET_TESTING, MakeDataPacketTest) {
    const size_t bufferLen = 1024;  
    const size_t dataLen = 512;     
    uint8_t sendBuffer[bufferLen];
    uint8_t data[dataLen] = {};
    uint16_t blockNum = 42;  //random block number

    // function call
    int result = makeDataPacket(sendBuffer, bufferLen, blockNum, data, dataLen);

    // Assertions
    EXPECT_GT(result, 0);   
    EXPECT_LE(result, static_cast<int>(bufferLen));  
}

// ACK packet
TEST(TFTP_ACK_PACKET_TESTING, ACKPacket){
    TftpOpcode ACK_Code = TFTP_OPCODE_ACK;

    uint8_t ackPacket[TFTP_MAX_PACKET_SIZE];
    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    int ret;
    
    // Generating the expected ACK packet
    int indx = 0;
    int blockNum = 1; 
    
    // OPCODE
    uint16_t networkOpcode = htons(ACK_Code);
    expected[indx] = (uint8_t)(networkOpcode & 0xFF);
    expected[indx+1] = (uint8_t)(networkOpcode>>8 & 0xFF);
    indx += 2;

    // Block no.
    uint16_t networkBlockNum = htons(blockNum);
    expected[indx] = (uint8_t)(networkBlockNum & 0xFF);
    expected[indx+1] = (uint8_t)((networkBlockNum>>8) & 0xFF);
    
    // function call
    ret = makeACKPacket(ackPacket, sizeof(ackPacket), blockNum);
    
    // Assertions
    ASSERT_GT(ret, 0);
    ASSERT_EQ(memcmp(expected, ackPacket, ret), 0);
}


// =================================================================================================

class MockLogger {
public:
    MOCK_METHOD(void, Log, (const std::string&), (const));
};


// File Availability
TEST(STARKFileAvailabilityTest, IsFileAvailableTest) {
    // Mock the logger
    MockLogger mockLogger;

    // Singleton instance of STARK class exists
    STARK::getInstance().setRootDir("./testfiles/");

    // Test case 1: File is available
    std::string availableFileName = "temp1.txt";
    std::string availableFilePath = STARK::getInstance().root_dir + availableFileName;
    // std::cout<<availableFilePath<<std::endl;
    std::ifstream availableFile(availableFilePath.c_str());
    availableFile.close();

    // Test function
    ASSERT_TRUE(STARK::getInstance().isFileAvailable(availableFileName));

    // Test case 2: File is not available
    std::string notAvailableFileName = "temp2.txt";
    std::remove((STARK::getInstance().root_dir + notAvailableFileName).c_str());

    // Test function
    ASSERT_FALSE(STARK::getInstance().isFileAvailable(notAvailableFileName));

    // Test case 3: Empty file name
    std::string emptyFileName = "";

    // Test the function
    ASSERT_FALSE(STARK::getInstance().isFileAvailable(emptyFileName));
}

TEST(STARKFileDeletableTest, ValidDeletableFile) {
    STARK::getInstance().setRootDir("./testfiles/");
    const std::string validFileName = "temp1.txt";
    TftpErrorCode errorCode;

    bool result = STARK::getInstance().isFileDeletable(validFileName, errorCode);

    // Check if the file is deletable
    EXPECT_TRUE(result);
}

TEST(STARKFileDeletableTest, NotFoundFile) {
    STARK::getInstance().setRootDir("./testfiles/");
    const std::string notFoundFileName = "var.txt";
    TftpErrorCode errorCode;

    bool result = STARK::getInstance().isFileDeletable(notFoundFileName, errorCode);

    // Check if the file is not deletable due to not being found
    EXPECT_FALSE(result);
}

TEST(STARKFileDeletableTest, EmptyFileName) {
    STARK::getInstance().setRootDir("./testfiles/");
    const std::string emptyFileName = "";
    TftpErrorCode errorCode;

    bool result = STARK::getInstance().isFileDeletable(emptyFileName, errorCode);

    // Check if the file is not deletable due to an empty file name
    EXPECT_FALSE(result);
}