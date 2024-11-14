#include <algorithm>
#include <iostream>

#include "MarketDataServer.h"
#include "PortfolioUtils.h"

using namespace ::minirisk;

void run(const string &portfolio_file, const string &risk_factors_file)
{
    // load the portfolio from file
    portfolio_t portfolio = load_portfolio(portfolio_file);
    // save and reload portfolio to implicitly test round trip serialization
    save_portfolio("portfolio.tmp", portfolio);
    portfolio.clear();
    portfolio = load_portfolio("portfolio.tmp");

    // display portfolio
    print_portfolio(portfolio);

    // get pricers
    std::vector<ppricer_t> pricers(get_pricers(portfolio));

    // initialize market data server
    std::shared_ptr<const MarketDataServer> mds(new MarketDataServer(risk_factors_file));

    // Init market object
    Date today(2017, 8, 5);
    Market mkt(mds, today);

    // Price all products. Market objects are automatically constructed on demand,
    // fetching data as needed from the market data server.
    {
        auto prices = compute_prices(pricers, mkt);

        // Calculate total and identify failed trades
        double total = 0.0;
        std::vector<std::pair<size_t, std::string>> failed_trades;

        for (size_t i = 0; i < prices.size(); ++i)
        {
            if (std::isnan(prices[i].first))
            {
                failed_trades.emplace_back(i, prices[i].second);
            }
            else
            {
                total += prices[i].first;
            }
        }

        // Display the total value of the portfolio
        std::cout << "Total PV of successfully priced trades: " << total << "\n";

        // Display the number of failed trades and their error messages
        if (!failed_trades.empty())
        {
            std::cout << "Number of trades that failed to price: " << failed_trades.size() << "\n";
            for (const auto &ft : failed_trades)
            {
                std::cout << "Trade index " << ft.first << " failed to price with error: " << ft.second << "\n";
            }
        }

        // Display PV
        print_price_vector("PV", prices);
    }

    // disconnect the market (no more fetching from the market data server allowed)
    mkt.disconnect();

    // display all relevant risk factors
    {
        std::cout << "Risk factors:\n";
        auto tmp = mkt.get_risk_factors(".+");
        for (const auto &iter : tmp)
            std::cout << iter.first << "\n";
        std::cout << "\n";
    }

    {
        // Compute PV01 bucketed (sensitivity with respect to interest rate dV/dr)
        std::vector<std::pair<string, portfolio_values_t>> pv01_bucketed = compute_pv01_bucketed(pricers, mkt);

        // Display PV01 per currency for bucketed
        for (const auto &g : pv01_bucketed)
        {
            print_price_vector("PV01 Bucketed " + g.first, g.second);
        }

        // Compute PV01 parallel
        std::vector<std::pair<string, portfolio_values_t>> pv01_parallel = compute_pv01_parallel(pricers, mkt);

        // Display PV01 per currency for parallel
        for (const auto &g : pv01_parallel)
        {
            print_price_vector("PV01 Parallel " + g.first, g.second);
        }
    }
}

void usage()
{
    std::cerr
        << "Invalid command line arguments\n"
        << "Example:\n"
        << "DemoRisk -p portfolio.txt -f risk_factors.txt\n";
    std::exit(-1);
}

int main(int argc, const char **argv)
{
    // parse command line arguments
    string portfolio, riskfactors;
    if (argc % 2 == 0)
        usage();
    for (int i = 1; i < argc; i += 2)
    {
        string key(argv[i]);
        string value(argv[i + 1]);
        if (key == "-p")
            portfolio = value;
        else if (key == "-f")
            riskfactors = value;
        else
            usage();
    }
    if (portfolio == "" || riskfactors == "")
        usage();

    try
    {
        run(portfolio, riskfactors);
        return 0; // report success to the caller
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return -1; // report an error to the caller
    }
    catch (...)
    {
        std::cerr << "Unknown exception occurred\n";
        return -1; // report an error to the caller
    }
}
