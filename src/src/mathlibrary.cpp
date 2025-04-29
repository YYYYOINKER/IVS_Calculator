//mathlibrary.cpp

#include "../include/mathlibrary.h"

/**
 * @brief Implementation of addition operation
 */
double Calculator::add(double a, double b) {
	return a + b;
}

/**
 * @brief Implementation of subtraction operation
 */
double Calculator::sub(double a, double b) {
	return a - b;
}

/**
 * @brief Implementation of multiplication operation
 */
double Calculator::mul(double a, double b) {
	return a * b;
}

/**
 * @brief Implementation of division operation
 */
double Calculator::div(double a, double b) {
	if(b == 0) throw std::runtime_error("Division by zero");
	return a/b;
}

/**
 * @brief Implementation of integer check
 */
bool Calculator::isInteger(double a) {
    long long intPart = static_cast<long long>(a);
    const double epsilon = 1e-12;
    double diff = a - intPart;
    return (diff < epsilon && diff > -epsilon);
}

/**
 * @brief Implementation of factorial calculation
 */
double Calculator::fact(double a) {
	if(a < 0) throw std::invalid_argument("Factorial not defined for negative numbers");
	if (!isInteger(a)) throw std::invalid_argument("Factorial requires integer value");
	if (a == 0 || a == 1) return 1.0;

	double result = 1.0;
	for(double i = 2.0; i <= a; i += 1.0) {
		result *= i;
		if (result > 1e308) throw std::overflow_error("Factorial overflow: result too large");
	}
	return result;
}

/**
 * @brief Implementation of power calculation
 */
double Calculator::power(double a, double b) {
    if(b == 0) return 1.0;
	if(!isInteger(b) || b < 0) throw std::invalid_argument("Only accepts natural exponent values");
    double result = 1.0;
    for(int i = 1; i <= b; i++) {
        result *= a;
    }
    return result;
}

/**
 * @brief Implementation of root calculation using Newton-Raphson method
 */
double Calculator::root(double a, double b) {
	if(!isInteger(b) || (b <= 0)) throw std::invalid_argument("Only accepts natural numbers");
    if((a < 0) && (((int)b%2) == 0)) throw std::invalid_argument("Even root of negative number is invalid");

    // Newton-Raphson method to approximate b-th root of a
    double x = a / 2.0; // initial guess
    const double epsilon = 1e-10;
    const int max_iterations = 1000;

    for(int i = 0; i < max_iterations; i++) {
        double prev = x;
        x = ((b-1) * x + a / power(x, b-1)) / b;
        if ((prev - x > -epsilon) && (prev - x < epsilon)) break;
    }
    return x;
}

/**
 * @brief Implementation of modulo operation
 */
double Calculator::modulo(double a, double b) {
	if(b == 0) throw std::runtime_error("Division by zero");
    if(!isInteger(a) || !isInteger(b)) throw std::invalid_argument("Only accepts integer values");
    int result = static_cast<int>(a) % static_cast<int>(b);
    if(result < 0) result += std::abs(static_cast<int>(b));
    return static_cast<double>(result);
}
