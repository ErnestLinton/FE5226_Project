#pragma once

#include "Trade.h"
#include "Market.h"
#include <vector>
#include <string>
#include <utility>

namespace minirisk
{

// Type definitions
typedef std::vector<std::pair<double, std::string>> portfolio_values_t;
typedef std::vector<ptrade_t> portfolio_t;
typedef std::shared_ptr<ITradePricer> ppricer_t;

// Functions for portfolio processing
void print_portfolio(const portfolio_t &portfolio);
std::vector<ppricer_t> get_pricers(const portfolio_t &portfolio);
portfolio_values_t compute_prices(const std::vector<ppricer_t> &pricers, Market &mkt);

// Compute total portfolio value and return the total and errors (if any)
std::pair<double, std::vector<std::pair<size_t, std::string>>> portfolio_total(const portfolio_values_t &values);

// Compute PV01 sensitivities for a parallel shift
std::vector<std::pair<std::string, portfolio_values_t>> compute_pv01_parallel(const std::vector<ppricer_t> &pricers, Market &mkt);

// Compute PV01 sensitivities for a bucketed shift
std::vector<std::pair<std::string, portfolio_values_t>> compute_pv01_bucketed(const std::vector<ppricer_t> &pricers, Market &mkt);

// Loading and saving portfolio functions
ptrade_t load_trade(my_ifstream &is);
void save_portfolio(const std::string &filename, const std::vector<ptrade_t> &portfolio);
std::vector<ptrade_t> load_portfolio(const std::string &filename);

// Utility function to print portfolio values
void print_price_vector(const std::string &name, const portfolio_values_t &values);

} // namespace minirisk
