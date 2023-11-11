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
#include "tftp_packets.hpp"

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
    std::make_tuple(TFTP_ERROR_NOT_DEFINED, "error packet 1"),
    std::make_tuple(TFTP_ERROR_FILE_NOT_FOUND, "error packet 3"),
    std::make_tuple(TFTP_ERROR_ACCESS_VIOLATION, "error packet 5"),
    std::make_tuple(TFTP_ERROR_DISK_FULL, "error packet 7")
};

INSTANTIATE_TEST_SUITE_P(ErrorPackets, TftpPosErrorPacketTesting, testing::ValuesIn(errorTestCases));


// Fail
TEST(TFTP_PACKET_TESTING, ErrorPacket4){
    TftpErrorCode errorCode = TFTP_ERROR_FILE_NOT_FOUND;

    uint8_t errorPacket[TFTP_MAX_PACKET_SIZE];
    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    const char* errMsg = "error packet 4";
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
    
    ret = makeErrorPacket(errorPacket, sizeof(errorPacket), errorCode, errMsg);

    // Assertions 
    ASSERT_GT(ret, 0);
    ASSERT_NE(memcmp(expected, errorPacket, ret), 0);
}

// Fail
TEST(TFTP_PACKET_TESTING, ErrorPacket6){
    TftpErrorCode errorCode = TFTP_ERROR_ACCESS_VIOLATION;

    uint8_t errorPacket[TFTP_MAX_PACKET_SIZE];
    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    const char* errMsg = "error packet 6";
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
    
    ret = makeErrorPacket(errorPacket, sizeof(errorPacket), errorCode, errMsg);
    
    // Assertions
    ASSERT_GT(ret, 0);
    ASSERT_NE(memcmp(expected, errorPacket, ret), 0);
}