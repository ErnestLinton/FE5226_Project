#include "CurveDiscount.h"
#include "Market.h"
#include "Streamer.h"

#include <cmath>


namespace minirisk {

CurveDiscount::CurveDiscount(Market *mkt, const Date& today, const string& curve_name)
    : m_today(today)
    , m_name(curve_name)
    , m_rate(mkt->get_yield(curve_name.substr(ir_curve_discount_prefix.length(),3)))
{
}

double  CurveDiscount::df(const Date& t) const
{
    MYASSERT((!(t < m_today)), "cannot get discount factor for date in the past: " << t);
    double dt = time_frac(m_today, t);
    return std::exp(-m_rate * dt);
}

} // namespace minirisk
