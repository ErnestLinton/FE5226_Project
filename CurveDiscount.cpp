#include "CurveDiscount.h"
#include "Market.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace minirisk
{

    CurveDiscount::CurveDiscount(Market *mkt, const Date &today, const string &curve_name)
        : m_today(today), m_name(curve_name)
    {
        // Extract currency code assuming curve_name format is "IR.USD"
        std::string currency = curve_name.substr(curve_name.size() - 3);

        // Construct regex pattern to match relevant rates for the curve
        std::string pattern = "^IR\\.[0-9]+[DWMY]\\." + currency + "$";

        // Retrieve all relevant risk factors from the market
        auto risk_factors = mkt->get_risk_factors(pattern);

        // Convert tenors to days and store rates
        for (const auto &rf : risk_factors)
        {
            double tenor_days = convert_tenor_to_days(rf.first); // Convert tenor string to days
            m_rates.emplace_back(tenor_days, rf.second);
        }

        // Sort rates based on tenor days
        std::sort(m_rates.begin(), m_rates.end());

        // Precompute local rates for interpolation
        compute_local_rates();
    }

    double CurveDiscount::convert_tenor_to_days(const std::string &tenor) const
    {
        // Assume tenor format is "1W", "1M", "2Y", etc.
        char unit = tenor.back();                                 // Get the last character, which denotes the unit
        int value = std::stoi(tenor.substr(0, tenor.size() - 1)); // Extract the numeric value

        switch (unit)
        {
        case 'D':
            return value;
        case 'W':
            return value * 7;
        case 'M':
            return value * 30;
        case 'Y':
            return value * 365;
        default:
            throw std::invalid_argument("Unsupported tenor unit: " + std::string(1, unit));
        }
    }

    void CurveDiscount::compute_local_rates()
    {
        for (size_t i = 0; i < m_rates.size() - 1; ++i)
        {
            double Ti = m_rates[i].first;
            double Ti1 = m_rates[i + 1].first;

            double ri = m_rates[i].second;
            double ri1 = m_rates[i + 1].second;

            double delta_t = Ti1 - Ti;
            if (delta_t <= 0)
            {
                throw std::logic_error("Invalid tenor range, delta_t must be positive");
            }

            // Compute local rate to ensure continuity in discount factors
            double local_rate = (ri1 * Ti1 - ri * Ti) / delta_t;
            m_local_rates.push_back(local_rate);
        }
    }

    double CurveDiscount::df(const Date &t) const
    {
        // Check if date is within valid range
        if (t < m_today)
        {
            throw std::out_of_range("Cannot get discount factor for date in the past: " + t.to_string());
        }

        double days_from_today = static_cast<double>(t - m_today);

        // Use binary search to find the appropriate interval
        auto it = std::lower_bound(m_rates.begin(), m_rates.end(), days_from_today,
                                   [](const std::pair<double, double> &rate, double days)
                                   {
                                       return rate.first < days;
                                   });

        if (it == m_rates.end())
        {
            throw std::out_of_range("Date exceeds the maximum tenor in the curve");
        }

        size_t index = std::distance(m_rates.begin(), it);
        if (index == 0)
            index = 1; // If it's the first point, use the first interval

        double Ti = m_rates[index - 1].first;
        double ri = m_rates[index - 1].second;
        double local_rate = m_local_rates[index - 1];
        double dt = days_from_today - Ti;

        // Compute discount factor using the specified formula
        return std::exp(-ri * Ti / 365.0 - local_rate * dt / 365.0);
    }
} // namespace minirisk
