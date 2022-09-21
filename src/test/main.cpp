#include <gtest/gtest.h>

#include "tst_hash.h"
#include "tst_hash_roll.h"

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}