#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Date.h"

using namespace minirisk;

void test1() {
    std::srand(static_cast<unsigned>(std::time(nullptr))); // Randomize the seed
    unsigned seed = static_cast<unsigned>(std::time(nullptr));
    std::cout << "Random seed: " << seed << std::endl;

    for (int i = 0; i < 1000; ++i) {
        unsigned year = Date::first_year + (std::rand() % (Date::last_year - Date::first_year));
        unsigned month = 1 + (std::rand() % 12);
        unsigned day = 1 + (std::rand() % 31); // Intentionally generate potentially invalid days

        try {
            minirisk::Date invalidDate(year, month, day);
            std::cerr << "Test failed: Created an invalid date " << year << "-" << month << "-" << day << std::endl;
            throw std::runtime_error("Invalid date did not trigger an exception");
        } catch (const std::exception& e) {
            // Expected outcome: an exception should be thrown for invalid dates
        }
    }
}

void test2() {
    for (unsigned year = Date::first_year; year < Date::last_year; ++year) {
        for (unsigned month = 1; month <= 12; ++month) {
            unsigned max_day = Date::days_in_month[month - 1];
            if (month == 2 && Date::is_leap_year(year)) {
                max_day += 1; // Adjust for leap year
            }
            for (unsigned day = 1; day <= max_day; ++day) {
                minirisk::Date date(year, month, day);
                unsigned serial = date.get_serial();

                unsigned y, m, d;
                minirisk::Date::serial_to_date(serial, y, m, d);

                if (year != y || month != m || day != d) {
                    std::cerr << "Test failed: Conversion mismatch for date " << year << "-" << month << "-" << day << std::endl;
                    throw std::runtime_error("Date-to-serial and serial-to-date conversion failed");
                }
            }
        }
    }
}

void test3()
{
}

int main()
{
    test1();
    test2();
    test3();
    return 0;
}

