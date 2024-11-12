#include "TradePayment.h"
#include "PricerPayment.h"

namespace minirisk {

ppricer_t TradePayment::pricer() const
{
    return ppricer_t(new PricerPayment(*this));
}

} // namespace minirisk
