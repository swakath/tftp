#include <gtest/gtest.h>
#include "tftp_common.hpp"
INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}