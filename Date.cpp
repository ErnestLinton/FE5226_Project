#include <iomanip>

#include "Date.h"

namespace minirisk {

struct DateInitializer : std::array<unsigned, Date::n_years>
{
    DateInitializer()
    {
        for (unsigned i = 0, s = 0, y = Date::first_year; i < size(); ++i, ++y) {
            (*this)[i] = s;
            s += 365 + (Date::is_leap_year(y) ? 1 : 0);
        }
    }
};

const std::array<unsigned, 12> Date::days_in_month = { {31,28,31,30,31,30,31,31,30,31,30,31} };
const std::array<unsigned, 12> Date::days_ytd{ {0,31,59,90,120,151,181,212,243,273,304,334} };
const std::array<unsigned, Date::n_years> Date::days_epoch(static_cast<const std::array<unsigned, Date::n_years>&>(DateInitializer()));

/* The function checks if a given year is a leap year.
    Leap year must be a multiple of 4, but it cannot be a multiple of 100 without also being a multiple of 400.
*/
bool Date::is_leap_year(unsigned year)
{
    return ((year % 4 != 0) ? false : (year % 100 != 0) ? true : (year % 400 != 0) ? false : true);
}

// The function pads a zero before the month or day if it has only one digit.
std::string Date::padding_dates(unsigned month_or_day)
{
    std::ostringstream os;
    os << std::setw(2) << std::setfill('0') << month_or_day;
    return os.str();
}

void Date::check_valid(unsigned y, unsigned m, unsigned d)
{
    MYASSERT(y >= first_year, "The year must be no earlier than year " << first_year << ", got " << y);
    MYASSERT(y < last_year, "The year must be smaller than year " << last_year << ", got " << y);
    MYASSERT(m >= 1 && m <= 12, "The month must be a integer between 1 and 12, got " << m);
    unsigned dmax = days_in_month[m - 1] + ((m == 2 && is_leap_year(y)) ? 1 : 0);
    MYASSERT(d >= 1 && d <= dmax, "The day must be a integer between 1 and " << dmax << ", got " << d);
}

// Compute serial number (ie. days since 1-Jan-1900)
unsigned Date::compute_serial(unsigned year, unsigned month, unsigned day) {
    unsigned days = days_epoch[year - first_year];
    days+= days_ytd[month -1] + ((month > 2 && is_leap_year(year)) ? 1:0 ) + (day-1);
    return days;
}

// Convert serial number to a (year, month, day) date
void Date::serial_to_date(unsigned serial, unsigned& year, unsigned& month, unsigned& day) {
    // Find year by iterating through days_epoch array
    unsigned y = first_year;
    while (serial >= days_epoch[y-first_year + 1]) ++y;
    year = y;

    // Find remaining days in year
    unsigned days_in_year = serial - days_epoch[year - first_year];

    // Find month by iterating through days_ytd
    month = 1;
    while(days_in_year >= days_ytd[month]) ++month;
    // Adjust for leap years if date is after Feb
    if (month > 2 && is_leap_year(year)) {
        days_in_year-=1; // adjust for 29 feb
        if (days_in_year == 59) { // check for 29 Feb itself
            month = 2;
            day = 29;
        }
    }
        
    // Calculate Day within month
    day = days_in_year - days_ytd[month - 1] + 1;
        
    }

/*  The function calculates the distance between two Dates.
    d1 > d2 is allowed, which returns the negative of d2-d1.
*/
long operator-(const Date& d1, const Date& d2) {
    return static_cast<long>(d1.get_serial()) - static_cast<long>(d2.get_serial());
}

} // namespace minirisk

