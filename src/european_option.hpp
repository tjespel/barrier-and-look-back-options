#ifndef EUROPEAN_OPTION_H   // header guard
#define EUROPEAN_OPTION_H
#include <string> // For user instructions in greeks
#ifndef M_PI
#define M_PI 3.1415926535898
#endif

#include "random_normal.hpp" // For random normal functions

/*
Black-Scholes European Call Option Class
This class encapsulates the BSCall.
*/

class european_option{
    // VARIABLES
protected:
    double St, K, r, sigma, T, t, d1, d2;
    double tau, discount;
    int number_iterations;
    random_normal random_sample;
    vector<double> stock_price_at_maturity;

    // UTILITY FUNCTIONS

    // Computes the stock price using the direct method
    double stock_price_single();
    double stock_price_single(double time_final, double time_inital);
    double stock_price_single(double S0, double T1, double t1);

    // Computes the stock price using the direct method with the random variable as an input
    double stock_price_single(double Z);
    // Computes the call payoff using a single stock price (random)
    double call_payoff_single();
    // Computes the call payoff using a single stock price with the random variable as an input
    double call_payoff_single(double Z);
    // Computes d1 for a Black-Scholes closed-form formula
    double d1_calculate();
    // Computes d1 for a Black-Scholes closed-form formula (with given parameters)
    double d1_calculate(double St_given, double strike_given);
    // Computes d1 for a Black-Scholes closed-form formula, used for look-back option
    double d1_calculate(double a, double b, double c);
    // Computes d2 for a Black-Scholes closed-form formula
    double d2_calculate();
    // Computes d2 for a Black-Scholes closed-form formula (with given parameters)
    double d2_calculate(double St_given, double strike_given);

    // MAIN FUNCTIONS

    // Computes delta using closed-form formula
    double delta_theoretic();
    double delta_theoretic_call(double S0, double k); //For barrier_option
    // Computes gamma using closed-form formula
    double gamma_theoretic();
    double gamma_theoretic_call(double S0, double k); // For barrier_option
    // Computes vega using closed-form formula
    double vega_theoretic();
    double vega_theoretic_call(double S0, double k); // For barrier_option
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
    // Computes vega using likelihood ratio method
    double vega_lr();
    // Computes vega using pathwise derivatives estimates method
    double vega_pw();

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
        discount = exp(-r*tau);
        number_iterations = number_iterations_approximation;
        d1=d1_calculate();
        d2=d2_calculate();
        random_sample.generate(number_iterations);
        stock_price_at_maturity.resize(number_iterations, 0);
        for (long int i = 0; i < number_iterations; i++) stock_price_at_maturity[i]=stock_price_single(random_sample[i]);
    }


    // ACCESS FUNCTIONS

    // Computes the call payoff using a closed-form formula
    double payoff_theoretic();
    // Computes the call payoff, using the parameters given
    double payoff_theoretic(double initial_value_given, double strike_given);

    // Computes the call price by averaging (expectation) several (random) single call payoffs
    double price();
    /* Computes the delta according to the user input:
    - 'th' for result using closed-form Black-Scholes formula
    - 'pw' for pathwise derivatives estimates method
    - 'lr' for likelihood ratios method
    'th' is built as default
    */
    double delta(std::string method);
    double delta();
    /* Computes the gamma according to the user input:
    - 'th' for result using closed-form Black-Scholes formula
    - 'lrpw' for likelihood ratios - pathwise derivatives estimates method
    - 'pwlr' for likelihood ratios - pathwise derivatives estimates method
    - 'lrlr' for double likelihood ratios method
    'th' is built as default
    */
    double gamma(std::string method);
    double gamma();
    /* Computes the vega according to the user input:
    - 'th' for result using closed-form Black-Scholes formula
    - 'pw' for pathwise derivatives estimates method
    - 'lr' for likelihood ratios method
    'th' is built as default
    */
    double vega(std::string method);
    double vega();

    // SERVICE FUNCTIONS

};

#endif
