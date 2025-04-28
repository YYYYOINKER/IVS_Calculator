#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdexcept>
#include <cmath>

class Calculator{
	public:
		Calculator() = default;

		double add(double a, double b);
		double sub(double a, double b);
		double mul(double a, double b);
		double div(double a, double b);
		double fact(double a);
		double power(double a, double b);
		double root(double a, double b);
		int modulo(int a, int b);
};

#endif
