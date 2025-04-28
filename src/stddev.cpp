#include <iostream>
#include <iomanip>
#include "include/mathlibrary.h"

/**
 * @brief Vypočíta výberovú smerodajnú odchýlku optimalizovaným spôsobom
 * @return Výberová smerodajná odchýlka zaokrúhlená na 4 desatinné miesta
 */
double calculate_stddev() {
    Calculator calc;
    double x;
    size_t N = 0;
    double sum = 0.0;
    double sum_squares = 0.0;
    
    // Online algoritmus - počítame priebežne bez ukladania všetkých čísel
    while (std::cin >> x) {
        sum = calc.add(sum, x);
        sum_squares = calc.add(sum_squares, calc.mul(x, x));
        N++;
    }
    
    if (N < 2) {
        return 0.0;
    }
    
    // Numericky stabilnejší výpočet rozptylu
    const double mean = calc.div(sum, static_cast<double>(N));
    const double variance = calc.div(
        calc.sub(sum_squares, calc.mul(static_cast<double>(N), calc.mul(mean, mean))),
        static_cast<double>(N - 1)
    );
    
    return calc.root(variance, 2.0);
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
