#pragma once

#include "Macros.h"
#include <string>
#include <array>

namespace minirisk {

    struct Date {
        public:
            static const unsigned first_year = 1900;
            static const unsigned last_year = 2200;
            static const unsigned n_years = last_year - first_year;
            
        private:
            static std::string padding_dates(unsigned);

            // number of days since 1-Jan-1900
            unsigned serial;

            // convert date to serial representation
            static unsigned compute_serial(unsigned year, unsigned month, unsigned day);

            // number of days elapsed from beginning of the year
            unsigned day_of_year() const;

            // convert serial number back to day, month, year
            static void serial_to_date(unsigned serial, unsigned& year, unsigned& month, unsigned& day);

            friend long operator-(const Date& d1, const Date& d2);

            static const std::array<unsigned, 12> days_in_month;  // num of days in month M in a normal year
            static const std::array<unsigned, 12> days_ytd;      // num of days since 1-jan to 1-M in a normal year
            static const std::array<unsigned, n_years> days_epoch;   // num of days since 1-jan-1900 to 1-jan-yyyy (until 2200)

        public:
            // Default constructor
            Date() : serial(compute_serial(1970, 1, 1)) {} // why is this set to 1970?

            // Constructor where the input value is checked.
            Date(unsigned year, unsigned month, unsigned day) {init(year, month, day);}

            void init(unsigned year, unsigned month, unsigned day) {
                check_valid(year, month, day);
                serial = compute_serial(year, month, day);
            }

            static void check_valid(unsigned y, unsigned m, unsigned d);

            bool operator<(const Date& d) const {return serial < d.serial;}
            bool operator==(const Date& d) const {return serial == d.serial;}
            bool operator>(const Date& d) const {return d < (*this);}

            // Serialization format based on serial
            unsigned get_serial() const {return serial;}

            static bool is_leap_year(unsigned yr);

            // Convert serial to YYYYMMDD format as a string
            std::string to_string(bool pretty = true) const {

                unsigned year, month, day;
                serial_to_date(serial, year, month, day);

                return pretty
                    ? std::to_string(day) + "-" + std::to_string(month) + "-" + std::to_string(year)
                    : std::to_string(year) + padding_dates(month) + padding_dates(day);
            }
        };

    long operator-(const Date& d1, const Date& d2);

    inline double time_frac(const Date& d1, const Date& d2) {
        return static_cast<double>(d2 - d1) / 365.0;
    }

    // Compute serial number (ie. days since 1-Jan-1900)
    unsigned Date::compute_serial(unsigned year, unsigned month, unsigned day) {
        unsigned days = Date::days_epoch[year - Date::first_year];
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

} // namespace minirisk
