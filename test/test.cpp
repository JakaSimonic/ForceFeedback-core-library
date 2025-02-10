#include <gtest/gtest.h>
#include "../UserInput.h"
#include "../FfbEngine.h"
#include "../FfbReportHandler.h"
#include <stdint.h>

uint64_t myTime()
{
    return 0;
}

// Sample function to test
int add(int a, int b)
{
    UserInput ui;
    FfbReportHandler ffh(myTime);
    FfbEngine ffE(ffh, ui, myTime);

    return a + b;
}

TEST(HelloWorldTest, TestAdd)
{
    EXPECT_EQ(add(2, 2), 4);
    EXPECT_EQ(add(-1, 1), 0);
}

TEST(HelloWorldTest, AnotherTest)
{
    EXPECT_TRUE(true);
}