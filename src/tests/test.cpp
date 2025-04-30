/**
 * @file test.cpp
 * @brief Unit tests for Calculator class using Google Test framework
 */

 #include <gtest/gtest.h>
 #include "../include/mathlibrary.h"
 
 TEST(CalculatorTest, Addition) {
     EXPECT_DOUBLE_EQ(10.0, Calculator::add(5.0, 5.0));
     EXPECT_DOUBLE_EQ(0.0, Calculator::add(0.0, 0.0));
     EXPECT_DOUBLE_EQ(-5.0, Calculator::add(-10.0, 5.0));
     EXPECT_DOUBLE_EQ(2.5, Calculator::add(1.25, 1.25));
 }
 
 TEST(CalculatorTest, Subtraction) {
     EXPECT_DOUBLE_EQ(0.0, Calculator::sub(5.0, 5.0));
     EXPECT_DOUBLE_EQ(-1.0, Calculator::sub(5.0, 6.0));
     EXPECT_DOUBLE_EQ(10.0, Calculator::sub(15.0, 5.0));
     EXPECT_DOUBLE_EQ(-2.5, Calculator::sub(1.25, 3.75));
 }
 
 TEST(CalculatorTest, Multiplication) {
     EXPECT_DOUBLE_EQ(15.0, Calculator::mul(5.0, 3.0));
     EXPECT_DOUBLE_EQ(0.0, Calculator::mul(0.0, 5.0));
     EXPECT_DOUBLE_EQ(-15.0, Calculator::mul(3.0, -5.0));
     EXPECT_DOUBLE_EQ(1.5625, Calculator::mul(1.25, 1.25));
 }
 
 TEST(CalculatorTest, Division) {
     EXPECT_NEAR(1.33333, Calculator::div(4.0, 3.0), 0.0001);
     EXPECT_DOUBLE_EQ(2.5, Calculator::div(5.0, 2.0));
     EXPECT_DOUBLE_EQ(-1.0, Calculator::div(-5.0, 5.0));
     EXPECT_THROW(Calculator::div(5.0, 0.0), std::runtime_error);
 }
 
 TEST(CalculatorTest, Factorial) {
     EXPECT_DOUBLE_EQ(120.0, Calculator::fact(5));
     EXPECT_DOUBLE_EQ(1.0, Calculator::fact(0));
     EXPECT_THROW(Calculator::fact(-5), std::invalid_argument);
     EXPECT_THROW(Calculator::fact(5.5), std::invalid_argument);
     EXPECT_THROW(Calculator::fact(1000), std::overflow_error);
 }
 
 TEST(CalculatorTest, Power) {
     EXPECT_DOUBLE_EQ(25.0, Calculator::power(5.0, 2.0));
     EXPECT_DOUBLE_EQ(1.0, Calculator::power(5.0, 0.0));
     EXPECT_THROW(Calculator::power(2.0, -3.0), std::invalid_argument);
     EXPECT_THROW(Calculator::power(5.0, 2.4), std::invalid_argument);
 }
 
 TEST(CalculatorTest, Root) {
     EXPECT_NEAR(2.0, Calculator::root(8, 3), 0.0001);
     EXPECT_NEAR(3.0, Calculator::root(9, 2), 0.0001);
     EXPECT_THROW(Calculator::root(-8, 2), std::invalid_argument);
     EXPECT_THROW(Calculator::root(8, 0), std::invalid_argument);
 }
 
 TEST(CalculatorTest, Modulo) {
     EXPECT_EQ(1, Calculator::modulo(10, 3));
     EXPECT_EQ(0, Calculator::modulo(10, 5));
     EXPECT_EQ(2, Calculator::modulo(-10, 3));
     EXPECT_THROW(Calculator::modulo(10.5, 3), std::invalid_argument);
     EXPECT_THROW(Calculator::modulo(10, 0), std::runtime_error);
 }
 
 TEST(CalculatorTest, IsInteger) {
     EXPECT_TRUE(Calculator::isInteger(5.0));
     EXPECT_TRUE(Calculator::isInteger(-3.0));
     EXPECT_FALSE(Calculator::isInteger(5.3));
     EXPECT_FALSE(Calculator::isInteger(-2.999));
     EXPECT_TRUE(Calculator::isInteger(0.0));
     EXPECT_TRUE(Calculator::isInteger(1e12));
     EXPECT_FALSE(Calculator::isInteger(1e12 + 0.0001));
 }
 
 /**
  * @brief Main function to run all tests
  */
 int main(int argc, char **argv) {
     ::testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
 }
 