#ifndef MATHLIBRARY_H
#define MATHLIBRARY_H

#include <iostream>
#include <stdexcept>

/**
 * @class Calculator
 * @brief A simple calculator class providing basic and advanced mathematical operations
 * 
 * This class implements various mathematical operations including addition, subtraction,
 * multiplication, division, factorial, power, root, and modulo operations.
 */
class Calculator {
	public:
		Calculator() = delete;
		
		/**
		 * @brief constant value of pi
		 */
		static constexpr double pi = 3.1415926536;
		/**
		 * @brief constant value of e
		 */
		static constexpr double e = 2.7182818285;

		/**
		 * @brief Adds two numbers
		 * @param a First operand
		 * @param b Second operand
		 * @return Sum of a and b
		 */
		static double add(double a, double b);
		
		/**
		 * @brief Subtracts two numbers
		 * @param a First operand
		 * @param b Second operand
		 * @return Difference between a and b (a - b)
		 */
		static double sub(double a, double b);
		
		/**
		 * @brief Multiplies two numbers
		 * @param a First operand
		 * @param b Second operand
		 * @return Product of a and b
		 */
		static double mul(double a, double b);
		
		/**
		 * @brief Divides two numbers
		 * @param a Dividend
		 * @param b Divisor
		 * @return Quotient of a divided by b
		 * @throws std::runtime_error if b is zero
		 */
		static double div(double a, double b);
		
		/**
		 * @brief Checks if a number is effectively an integer
		 * @param a Number to check
		 * @return true if the number is effectively an integer, false otherwise
		 */
		static bool isInteger(double a);
		
		/**
		 * @brief Calculates factorial of a number
		 * @param a Number to calculate factorial for
		 * @return Factorial of a
		 * @throws std::invalid_argument if a is negative or not an integer
		 * @throws std::overflow_error if factorial result is too large
		 */
		static double fact(double a);
		
		/**
		 * @brief Calculates power of a number
		 * @param a Base
		 * @param b Exponent
		 * @return a raised to the power of b
		 * @throws std::invalid_argument if exponent is negative or not an integer
		 */
		static double power(double a, double b);
		
		/**
		 * @brief Calculates nth root of a number
		 * @param a Radicand
		 * @param b Index of the root
		 * @return b-th root of a
		 * @throws std::invalid_argument if index is not a natural number or if trying to calculate even root of negative number
		 */
		static double root(double a, double b);
		
		/**
		 * @brief Calculates modulo of two numbers
		 * @param a Dividend
		 * @param b Divisor
		 * @return a modulo b
		 * @throws std::runtime_error if b is zero
		 * @throws std::invalid_argument if either a or b is not an integer
		 */
		static double modulo(double a, double b);
};

#endif
