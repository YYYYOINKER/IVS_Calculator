#include <gtest/gtest.h>
#include "../include/calculator.h"

class CalculatorTest : public ::testing::Test{
protected:
	Calculator calc;
};

TEST_F(CalculatorTest, Addition){
	EXPECT_DOUBLE_EQ(10.0, calc.add(5.0, 5.0));
	EXPECT_DOUBLE_EQ(0.0, calc.add(0.0, 0.0));
	EXPECT_DOUBLE_EQ(-5.0, calc.add(-10.0, 5.0));
	EXPECT_DOUBLE_EQ(2.5, calc.add(1.25, 1.25));
}

TEST_F(CalculatorTest, Subtraction){
	EXPECT_DOUBLE_EQ(0.0, calc.sub(5.0, 5.0));
	EXPECT_DOUBLE_EQ(-1.0, calc.sub(5.0, 6.0));
	EXPECT_DOUBLE_EQ(10.0, calc.sub(15.0, 5.0));
	EXPECT_DOUBLE_EQ(-2.5, calc.sub(1.25, 3.75));
}

TEST_F(CalculatorTest, Multiplication){
	EXPECT_DOUBLE_EQ(15.0, calc.mul(5.0, 3.0));
	EXPECT_DOUBLE_EQ(0.0, calc.mul(0.0, 5.0));
	EXPECT_DOUBLE_EQ(-15.0, calc.mul(3.0, -5.0));
	EXPECT_DOUBLE_EQ(1.5625, calc.mul(1.25, 1.25));
}

TEST_F(CalculatorTest, Division){
	EXPECT_NEAR(1.33333, calc.div(4.0, 3.0), 0.0001);
	EXPECT_DOUBLE_EQ(2.5, calc.div(5.0, 2.0));
	EXPECT_DOUBLE_EQ(-1.0, calc.div(-5.0, 5.0));
	EXPECT_THROW(calc.div(5.0, 0.0), std::runtime_error);
}

TEST_F(CalculatorTest, Factorial){
	EXPECT_DOUBLE_EQ(120.0, calc.fact(5));
	EXPECT_THROW(calc.fact(-5), std::invalid_argument);
	EXPECT_THROW(calc.fact(8.1), std::invalid_argument);
	EXPECT_THROW(calc.fact(1000), std::overflow_error);
}

TEST_F(CalculatorTest, Power){
	EXPECT_DOUBLE_EQ(25.0, calc.power(5.0, 2.0));
	EXPECT_DOUBLE_EQ(1.0, calc.power(5.0, 0.0));
	EXPECT_DOUBLE_EQ(0.25, calc.power(2.0, -2.0));
	EXPECT_THROW(calc.power(5.0, 2.4), std::invalid_argument);
}

TEST_F(CalculatorTest, Root){
	double result = calc.root(8, 3);
	EXPECT_NEAR(2.0, result, 0.0001);
	EXPECT_THROW(calc.root(8, 1.5), std::invalid_argument);
}
