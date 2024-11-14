#include "PortfolioUtils.h"
#include "Global.h"
#include "TradePayment.h"

#include <map>
#include <numeric>
#include <regex>
#include <vector>

namespace minirisk
{

    void print_portfolio(const portfolio_t &portfolio)
    {
        std::for_each(portfolio.begin(), portfolio.end(), [](auto &pt)
                      { pt->print(std::cout); });
    }

    std::vector<ppricer_t> get_pricers(const portfolio_t &portfolio)
    {
        std::vector<ppricer_t> pricers(portfolio.size());
        std::transform(portfolio.begin(), portfolio.end(), pricers.begin(),
                       [](auto &pt) -> ppricer_t
                       { return pt->pricer(); });
        return pricers;
    }

    portfolio_values_t compute_prices(const std::vector<ppricer_t> &pricers, Market &mkt)
    {
        portfolio_values_t prices;
        prices.reserve(pricers.size());

        for (const auto &pp : pricers)
        {
            try
            {
                double price = pp->price(mkt);
                prices.emplace_back(price, "");
            }
            catch (const std::exception &e)
            {
                prices.emplace_back(std::numeric_limits<double>::quiet_NaN(), e.what());
            }
        }

        return prices;
    }

    std::pair<double, std::vector<std::pair<size_t, std::string>>> portfolio_total(const portfolio_values_t &values)
    {
        double total = 0.0;
        std::vector<std::pair<size_t, std::string>> errors;

        for (size_t i = 0; i < values.size(); ++i)
        {
            if (std::isnan(values[i].first))
            {
                errors.emplace_back(i, values[i].second);
            }
            else
            {
                total += values[i].first;
            }
        }

        return std::make_pair(total, errors);
    }

    std::vector<std::pair<string, portfolio_values_t>> compute_pv01_parallel(const std::vector<ppricer_t> &pricers, Market &mkt)
    {
        std::vector<std::pair<string, portfolio_values_t>> pv01; // PV01 per trade
        std::map<string, Market::vec_risk_factor_t> base_map;

        const double bump_size = 0.01 / 100;

        // Filter risk factors related to IR using a regex pattern
        auto base = mkt.get_risk_factors(ir_rate_prefix + "\\d+[DWMY]\\.[A-Z]{3}");

        for (auto &b : base)
        {
            base_map[ir_rate_prefix + b.first.substr(b.first.length() - 3, 3)].push_back(b);
        }

        // Make a local copy of the Market object, because we will modify it applying bumps
        Market tmpmkt(mkt);

        // Compute prices for perturbed markets and aggregate results
        pv01.reserve(base_map.size());
        for (const auto &bm : base_map)
        {
            std::vector<std::pair<string, double>> bumped;
            pv01.push_back(std::make_pair(bm.first, portfolio_values_t(pricers.size())));
            portfolio_values_t pv_up, pv_dn;

            // Bump down and price
            for (size_t i = 0; i < bm.second.size(); ++i)
            {
                bumped.push_back(bm.second[i]);
                bumped[i].second = bm.second[i].second - bump_size;
            }
            tmpmkt.set_risk_factors(bumped);
            pv_dn = compute_prices(pricers, tmpmkt);

            // Bump up and price
            for (size_t i = 0; i < bm.second.size(); ++i)
            {
                bumped[i].second = bm.second[i].second + bump_size;
            }
            tmpmkt.set_risk_factors(bumped);
            pv_up = compute_prices(pricers, tmpmkt);

            // Restore original market state for next iteration
            for (size_t i = 0; i < bm.second.size(); ++i)
            {
                bumped[i].second = bm.second[i].second;
            }
            tmpmkt.set_risk_factors(bumped);

            // Compute estimator of the derivative via central finite differences
            double dr = 2.0 * bump_size;
            std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin(),
                           [dr](double hi, double lo) -> double
                           {
                               if (std::isnan(hi))
                                   return std::numeric_limits<double>::quiet_NaN();
                               else if (std::isnan(lo))
                                   return std::numeric_limits<double>::quiet_NaN();
                               else
                                   return (hi - lo) / dr;
                           });
        }

        return pv01;
    }

    std::vector<std::pair<string, portfolio_values_t>> compute_pv01_bucketed(const std::vector<ppricer_t> &pricers, Market &mkt)
    {
        const double bump_size = 0.01 / 100; // 1 basis point

        std::vector<std::pair<string, portfolio_values_t>> pv01; // PV01 per trade

        // Filter risk factors related to IR
        auto base = mkt.get_risk_factors(ir_rate_prefix + "\\d+[DWMY]\\.[A-Z]{3}");

        // Make a local copy of the Market object
        Market tmpmkt(mkt);

        // Compute prices for perturbed markets
        pv01.reserve(base.size());
        for (const auto &d : base)
        {
            portfolio_values_t pv_up, pv_dn;
            std::vector<std::pair<string, double>> bumped(1, d);
            pv01.push_back(std::make_pair(d.first, portfolio_values_t(pricers.size())));

            // Bump down and price
            bumped[0].second -= bump_size;
            tmpmkt.set_risk_factors(bumped);
            pv_dn = compute_prices(pricers, tmpmkt);

            // Bump up and price
            bumped[0].second += 2 * bump_size; // Bump up
            tmpmkt.set_risk_factors(bumped);
            pv_up = compute_prices(pricers, tmpmkt);

            // Restore original market state
            bumped[0].second = d.second;
            tmpmkt.set_risk_factors(bumped);

            // Compute central difference
            double dr = 2.0 * bump_size;
            std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin(),
                           [dr](double hi, double lo)
                           {
                               return (hi - lo) / dr;
                           });
        }

        return pv01;
    }

    ptrade_t load_trade(my_ifstream &is)
    {
        string name;
        ptrade_t p;

        // Read trade identifier
        guid_t id;
        is >> id;

        if (id == TradePayment::m_id)
            p.reset(new TradePayment);
        else
            THROW("Unknown trade type: " << id);

        p->load(is);

        return p;
    }

    void save_portfolio(const string &filename, const std::vector<ptrade_t> &portfolio)
    {
        // Test saving to file
        my_ofstream of(filename);
        for (const auto &pt : portfolio)
        {
            pt->save(of);
            of.endl();
        }
        of.close();
    }

    std::vector<ptrade_t> load_portfolio(const string &filename)
    {
        std::vector<ptrade_t> portfolio;

        // Test reloading the portfolio
        my_ifstream is(filename);
        while (is.read_line())
            portfolio.push_back(load_trade(is));

        return portfolio;
    }

    void print_price_vector(const string &name, const portfolio_values_t &values)
    {
        std::cout
            << "========================\n"
            << name << ":\n"
            << "========================\n"
            << "Total: " << portfolio_total(values).first
            << "\nErrors: " << portfolio_total(values).second.size()
            << "\n\n========================\n";

        for (size_t i = 0, n = values.size(); i < n; ++i)
        {
            if (std::isnan(values[i].first))
                std::cout << std::setw(5) << i << ": " << values[i].second << "\n";
            else
                std::cout << std::setw(5) << i << ": " << values[i].first << "\n";
        }

        std::cout << "========================\n\n";
    }

} // namespace minirisk