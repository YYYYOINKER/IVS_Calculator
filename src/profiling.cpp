#include "include/mathlibrary.h"

/**
 * @brief Calculates the sample standard deviation using an optimized method.
 * @return Sample standard deviation (not rounded).
 */
double calculate_stddev() {

    std::string token;
    double x;
    size_t N = 0;
    double sum = 0.0;
    double sum_squares = 0.0;

    // Read numbers from standard input
    while (std::cin >> token) {
        if (token == "e" || token == "end") {
            break;
        }

        // Try to convert input to number
        try {
            x = std::stod(token);
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid input: " << token << std::endl;
            continue;
        }

        sum = Calculator::add(sum, x);
        sum_squares = Calculator::add(sum_squares, Calculator::mul(x, x));
        N++;
    }

    // At least two numbers are needed to calculate standard deviation
    if (N < 2) {
        return 0.0;
    }

    double mean = Calculator::div(sum, static_cast<double>(N));
    double variance = Calculator::div(
        Calculator::sub(sum_squares, Calculator::mul(static_cast<double>(N), Calculator::mul(mean, mean))),
        static_cast<double>(N - 1)
    );

    return Calculator::root(variance, 2.0);
}

int main() {

    double stddev = calculate_stddev();

    std::cout << stddev << std::endl;

    return 0;
}

