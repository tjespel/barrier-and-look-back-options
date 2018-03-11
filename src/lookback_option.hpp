#ifndef LOOKBACK_OPTION_H
#define LOOKBACK_OPTION_H

#include "european_option.hpp" // Include the european_option class for inheritance
using namespace std;

/*
 *
This class encapsulates the Black-Scholes European Lookback Call Option with Fixed Strike
*/

class lookback_option: public european_option{
    // VARIABLES
protected:
    double maxmin;   // max/min observed up to present time (t)


    // UTILITY FUNCTIONS

    // Computes the cdf of the maximum of Brownian motion with drift
    double P_M(const double &m, const double &drift, const double &stime);
    //  Computes the pdf of the maximum of Brownian motion with drift
    double rho_M(const double &m, const double &drift, const double &stime);
    // Computes the pdf of the maximum of Brownian motion with drift, partially differentiated wrt drift
    double rho_pdrift(const double &m, const double &drift, const double &stime);
    // Computes the pdf of the maximum of Brownian motion with drift, partially differentiated wrt m
    double rho_prime(const double &m, const double &drift, const double &stime);
    // Newton–Raphson method
    double newtonmax(double r, double drift, double stime);


    // MAIN FUNCTIONS

    // Computes delta using closed-form formula
    double delta_theoretic() const;
    // Computes delta using likelihood ratio method
    double delta_lr();
    // Computes delta using pathwise derivatives estimates method
    double delta_pw();
    // Computes gamma using closed-form formula
    double gamma_theoretic() const;
    // Computes the gamma using double likelihood ratio method
    double gamma_lrlr();
    // Computes the gamma using pathwise derivatives - likelihood ratio method
    double gamma_pwlr();
    // Computes vega using closed-form formula
    double vega_theoretic() const;
    // Computes the vega using likelihood ratios method
    double vega_lr();
    // Computes the vega using pathwise derivatives estimates method
    double vega_pw();

public:
    // Constructor
    lookback_option( double initial_stock_price = 100.0, double strike = 100.0, double interest_rate = 0.05,
                     double volatility = 0.4,double time_final_T = 1, double time_initial_t = 0,
                     int number_iterations_approximation = 10000, double min_or_max_observed = 150): european_option(initial_stock_price, strike, interest_rate, volatility, time_final_T, time_initial_t, number_iterations_approximation){
        maxmin = min_or_max_observed; }


    // ACCESS FUNCTIONS

    // Computes price of European Lookback Call Option with Fixed Strike
    double price();
    double payoff_theoretic();
    /* Computes the delta according to the user input:
    - 'pw' for pathwise derivatives estimates method
    - 'lr' for likelihood ratios method
    'lr' is built as default
    */
    double delta(std::string method);
    double delta();
    /* Computes the gamma according to the user input:
    - 'pw' for pathwise derivatives estimates method
    - 'lr' for likelihood ratios method
    'lr' is built as default
    */
    double gamma(std::string method);
    double gamma();
    /* Computes the vega according to the user input:
    - 'pw' for pathwise derivatives estimates method
    - 'lr' for likelihood ratios method
    'lr' is built as default
    */
    double vega(std::string method);
    double vega();


    // SERVICE FUNCTIONS

};



#endif
