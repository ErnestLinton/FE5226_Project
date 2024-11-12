#pragma once

#include <memory>

#include "IObject.h"
#include "Market.h"

namespace minirisk {

struct IPricer : IObject
{
    virtual double price(Market& m) const = 0;
};


typedef std::shared_ptr<const IPricer> ppricer_t;

} // namespace minirisk
