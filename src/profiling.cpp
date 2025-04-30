#include <iomanip>
#include "include/mathlibrary.h"

/**
 * @brief Vypočíta výberovú smerodajnú odchýlku optimalizovaným spôsobom
 * @return Výberová smerodajná odchýlka zaokrúhlená na 4 desatinné miesta
 */
double calculate_stddev() {
 
    double x;
    size_t N = 0;
    double sum = 0.0;
    double sum_squares = 0.0;
    
    // Online algoritmus - počítame priebežne bez ukladania všetkých čísel
    while (std::cin >> x) {
        sum = Calculator::add(sum, x);
        sum_squares = Calculator::add(sum_squares, Calculator::mul(x, x));
        N++;
    }
    
    if (N < 2) {
        return 0.0;
    }
    
    // Numericky stabilnejší výpočet rozptylu
    const double mean = Calculator::div(sum, static_cast<double>(N));
    const double variance = Calculator::div(
        Calculator::sub(sum_squares, Calculator::mul(static_cast<double>(N), Calculator::mul(mean, mean))),
        static_cast<double>(N - 1)
    );
    
    return Calculator::root(variance, 2.0);
}

int main() {
    // Vypnutie synchronizácie pre rýchlejší vstup
    //std::ios_base::sync_with_stdio(false);
    
    double stddev = calculate_stddev();
    
    // Výstup s presným zaokrúhlením
    std::cout << std::fixed << std::setprecision(4);
    std::cout << stddev << std::endl;
    
    return 0;
}
