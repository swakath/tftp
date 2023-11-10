#include <gtest/gtest.h>
#include "tftp_packets.hpp"

TEST(TFTP_PACKET_TESTING, ErrorPacket1){
    TftpErrorCode errorCode = TFTP_ERROR_ACCESS_VIOLATION;
    uint8_t errorPacket[TFTP_MAX_PACKET_SIZE];

    uint8_t expected[TFTP_MAX_PACKET_SIZE];
    const char* errMsg = "error packet 1";
    int ret;
    
    int indx = 0;
    uint16_t temp;
    temp = htons(TFTP_OPCODE_ERROR);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx+1] = (uint8_t)(temp>>8 & 0xFF);
    indx += 2;

    temp = htons(errorCode);
    expected[indx] = (uint8_t)(temp & 0xFF);
    expected[indx+1] = (uint8_t)(temp>>8 & 0xFF);
    indx += 2;
    //memcpy(expected+indx,  errMsg, strlen(errMsg));
    //indx += strlen(errMsg);
    expected[indx] = 0x00;
    indx++;

    ret = makeErrorPacket(errorPacket, sizeof(errorPacket), errorCode, errMsg);
    ASSERT_GT(ret, 0);
    ASSERT_EQ(memcmp(expected, errorPacket, ret), 0);
}