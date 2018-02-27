#ifndef EUROPEAN_OPTION_H   // header guard
#define EUROPEAN_OPTION_H
#include <string> // For user instructions in greeks
#include "random_normal.hpp" // For random normal functions

/*
Black-Scholes European Call Option Class
This class encapsulates the BSCall.
*/

class european_option{
    // VARIABLES
    double St, K, r, sigma, T, t, d1, d2;
    double tau, discount;
    int number_iterations;

    // UTILITY FUNCTIONS

    // Computes the stock price using the direct method
    double stock_price_single();
    // Computes the stock price using the direct method with the random variable as an input
    double stock_price_single(double Z);
    // Computes the call payoff using a single stock price (random)
    double call_payoff_single();
    // Computes d1 for a Black-Scholes closed-form formula
    double d1_calculate();
    // Computes d2 for a Black-Scholes closed-form formula
    double d2_calculate();


    // MAIN FUNCTIONS

    // Computes delta using closed-form formula
    double delta_theoretic();
    // Computes gamma using closed-form formula
    double gamma_theoretic();
    // Computes vega using closed-form formula
    double vega_theoretic();
    // Computes delta using likelihood ratio method
    double delta_lr();
    // Computes delta using pathwise derivatives estimates method
    double delta_pw();
    // Computes gamma using likelihood ratio - pathwise derivatives method
    double gamma_lrpw();
    // Computes gamma using pathwise derivatives - likelihood ratio method
    double gamma_pwlr();
    // Computes gamma using double likelihood ratio method
    double gamma_lrlr();

public:
    // Constructor
    european_option(double initial_stock_price = 100.0, double strike = 100.0, double interest_rate = 0.05, double volatility = 0.4,double time_final_T = 1, double time_initial_t = 0, int number_iterations_approximation = 10000){
        St = initial_stock_price;
        K = strike;
        r = interest_rate;
        sigma = volatility;
        T = time_final_T;
        t = time_initial_t;
        tau = T - t;
        discount = exp(-r*(T - t));
        number_iterations = number_iterations_approximation;
        d1=d1_calculate();
        d2=d2_calculate();
    }


    // ACCESS FUNCTIONS

    // Computes the call payoff using a closed-form formula
    double payoff_theoretic();

    // Computes the call price by averaging (expectation) several (random) single call payoffs
    double price();
    /* Computes the delta according to the user input:
    - 'th' for result using closed-form Black-Scholes formula
    - 'pw' for pathwise derivatives estimates method
    - 'lr' for likelihood ratios method
    'lr' is regarded as most accurate, so built as default
    */
    double delta(std::string method);
    double delta();
    /* Computes the gamma according to the user input:
    - 'th' for result using closed-form Black-Scholes formula
    - 'lrpw' for likelihood ratios - pathwise derivatives estimates method
    - 'pwlr' for likelihood ratios - pathwise derivatives estimates method
    - 'lrlr' for double likelihood ratios method
    'pwlr' is regarded as most accurate, so built as default
    */
    double gamma(std::string method);
    double gamma();

    // SERVICE FUNCTIONS

};

#endif