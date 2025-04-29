/**
 * @file test.cpp
 * @brief Unit tests for Calculator class using Google Test framework
 */

#include <gtest/gtest.h>
#include "../include/mathlibrary.h"

/**
 * @class CalculatorTest
 * @brief Test fixture for Calculator class tests
 */

class CalculatorTest : public ::testing::Test {
protected:
    Calculator calc;
};

TEST_F(CalculatorTest, Addition) {
    EXPECT_DOUBLE_EQ(10.0, calc.add(5.0, 5.0));
    EXPECT_DOUBLE_EQ(0.0, calc.add(0.0, 0.0));
    EXPECT_DOUBLE_EQ(-5.0, calc.add(-10.0, 5.0));
    EXPECT_DOUBLE_EQ(2.5, calc.add(1.25, 1.25));
}

TEST_F(CalculatorTest, Subtraction) {
    EXPECT_DOUBLE_EQ(0.0, calc.sub(5.0, 5.0));
    EXPECT_DOUBLE_EQ(-1.0, calc.sub(5.0, 6.0));
    EXPECT_DOUBLE_EQ(10.0, calc.sub(15.0, 5.0));
    EXPECT_DOUBLE_EQ(-2.5, calc.sub(1.25, 3.75));
}

TEST_F(CalculatorTest, Multiplication) {
    EXPECT_DOUBLE_EQ(15.0, calc.mul(5.0, 3.0));
    EXPECT_DOUBLE_EQ(0.0, calc.mul(0.0, 5.0));
    EXPECT_DOUBLE_EQ(-15.0, calc.mul(3.0, -5.0));
    EXPECT_DOUBLE_EQ(1.5625, calc.mul(1.25, 1.25));
}

TEST_F(CalculatorTest, Division) {
    EXPECT_NEAR(1.33333, calc.div(4.0, 3.0), 0.0001);
    EXPECT_DOUBLE_EQ(2.5, calc.div(5.0, 2.0));
    EXPECT_DOUBLE_EQ(-1.0, calc.div(-5.0, 5.0));
    EXPECT_THROW(calc.div(5.0, 0.0), std::runtime_error);
}

TEST_F(CalculatorTest, Factorial) {
    EXPECT_DOUBLE_EQ(120.0, calc.fact(5));
    EXPECT_DOUBLE_EQ(1.0, calc.fact(0));
    EXPECT_THROW(calc.fact(-5), std::invalid_argument);
    EXPECT_THROW(calc.fact(5.5), std::invalid_argument);
    EXPECT_THROW(calc.fact(1000), std::overflow_error);
}

TEST_F(CalculatorTest, Power) {
    EXPECT_DOUBLE_EQ(25.0, calc.power(5.0, 2.0));
    EXPECT_DOUBLE_EQ(1.0, calc.power(5.0, 0.0));
    EXPECT_THROW(calc.power(2.0, -3.0), std::invalid_argument);
    EXPECT_THROW(calc.power(5.0, 2.4), std::invalid_argument);
}

TEST_F(CalculatorTest, Root) {
    EXPECT_NEAR(2.0, calc.root(8, 3), 0.0001);
    EXPECT_NEAR(3.0, calc.root(9, 2), 0.0001);
    EXPECT_THROW(calc.root(-8, 2), std::invalid_argument);
    EXPECT_THROW(calc.root(8, 0), std::invalid_argument);
}

TEST_F(CalculatorTest, Modulo) {
    EXPECT_EQ(1, calc.modulo(10, 3));
    EXPECT_EQ(0, calc.modulo(10, 5));
    EXPECT_EQ(2, calc.modulo(-10, 3));
    EXPECT_THROW(calc.modulo(10.5, 3), std::invalid_argument);
    EXPECT_THROW(calc.modulo(10, 0), std::runtime_error);
}

TEST_F(CalculatorTest, IsInteger) {
    EXPECT_TRUE(calc.isInteger(5.0));
    EXPECT_TRUE(calc.isInteger(-3.0));
    EXPECT_FALSE(calc.isInteger(5.3));
    EXPECT_FALSE(calc.isInteger(-2.999));
    EXPECT_TRUE(calc.isInteger(0.0));
    EXPECT_TRUE(calc.isInteger(1e12));
    EXPECT_FALSE(calc.isInteger(1e12 + 0.0001));
}

/**
 * @brief Main function to run all tests
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
