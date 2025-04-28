#include "../include/mathlibrary.h"
#include <stdexcept>
#include <cmath>

double Calculator::add(double a, double b){
	return a + b;
}

double Calculator::sub(double a, double b){
	return a - b;
}

double Calculator::mul(double a, double b){
	return a * b;
}

double Calculator::div(double a, double b){
	if(b == 0) throw std::runtime_error("Division by zero");

	return a/b;
}

double Calculator::fact(double a){
	if(a < 0) throw std::invalid_argument("Factorial not defined for negative numbers");

	if (floor(a) != a) throw std::invalid_argument("Factorial requires integer value");

	if (a == 0 || a == 1) return 1.0;

	double result = 1.0;

	for(double i = 2.0; i <= a; i += 1.0){
		result *= i;

		if (std::isinf(result)) throw std::overflow_error("Factorial overflow: result too large");
	}

	return result;
}

double Calculator::power(double a, double b){
	if(floor(b) != b) throw std::invalid_argument("Only accepts integer values");

	return pow(a, b);
}

double Calculator::root(double a, double b){
	if(floor(b) != b) throw std::invalid_argument("Only accepts integer values");

	return pow(a, 1/b);
}

int Calculator::modulo(int a, int b){
	if(b == 0) throw std::runtime_error("Division by zero");

	return a%b;
}
