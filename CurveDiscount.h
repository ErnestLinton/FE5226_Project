#pragma once

#include "Date.h"
#include "ICurve.h"
#include <string>
#include <vector>

namespace minirisk
{

    struct Market;

    struct CurveDiscount : ICurveDiscount
    {
        CurveDiscount(Market *mkt, const Date &today, const std::string &curve_name);

        virtual std::string name() const override { return m_name; }
        virtual Date today() const override { return m_today; }

        // Compute the discount factor for a given date
        virtual double df(const Date &t) const override;

    private:
        // Convert tenor string (e.g., "1W", "1M", "2Y") to number of days
        double convert_tenor_to_days(const std::string &tenor) const;

        // Precompute local rates for interpolation
        void compute_local_rates();

        Date m_today;                                   // Anchor date for the curve
        std::string m_name;                             // Curve name (e.g., "IR.USD")
        std::vector<std::pair<double, double>> m_rates; // Pairs of (days_from_today, rate)
        std::vector<double> m_local_rates;              // Precomputed local rates for interpolation
    };

} // namespace minirisk
