#include <gtest/gtest.h>
#include "strong.h"
#include <iostream>

TEST(SmokeTest, SmokeTest1) 
{
    std::cout << "Running SmokeTest::SmokeTest1\n";
    ASSERT_EQ(1, 1);
}