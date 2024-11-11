#include <typeinfo>
#include <iostream>
// #include <iomanip>
// #include <cstdlib>
#include <ctime>
#include <cmath>
#include "Date.h"

using namespace minirisk;
const std::array<unsigned, 12> days_in_month = { {31,28,31,30,31,30,31,31,30,31,30,31} };

// Verify that Date class constructor throws error for 1000 random invalid dates
void test1() {
    
    std::srand(static_cast<unsigned>(std::time(nullptr))); // Randomize the seed
    unsigned seed = static_cast<unsigned>(std::time(nullptr));
    std::cout << "Random seed: " << seed << std::endl;
    unsigned errors=0; // counts how many invalid dates Date class has identified

    // Generate 1000 invaid dates
    for (unsigned i = 0; i < 1000; ++i) {
        unsigned num = std::rand(), ones = num%10; // creates random number
        unsigned year, month, day;

        // create dates with invalid day, month or year
        switch(ones) {
            // test invalid days
            case 0: year = Date::first_year + (num % Date::n_years); month = 1 + num%12; day = 0; break;
            case 1: case 2: year = Date::first_year + (num % Date::n_years); month = 1 + num%12; day = (month==2 && Date::is_leap_year(year)) ? 30 : days_in_month[month-1] + 1; break;
            
            // test invalid months
            case 3: year = Date::first_year + (num % Date::n_years); month = 0; day = num % 28 + 1; break;
            case 4: case 5: year = Date::first_year + (num % Date::n_years); month = num % 12 + 13; day = num % 28 + 1; break;

            // test invalid years
            default: year = Date::first_year + Date::n_years/2 + pow(-1, ones)*(int)(Date::n_years/2 + num%(int)(Date::n_years/2)); month = 1 + num%12; day = num % 28 + 1; break;
        };
        
        // Attempt to construct the invalid date. Errors thrown by date class 
        try {
            Date invalidDate(year, month, day);
        } catch (const std::exception& e) {
            std::string error_type = typeid(e).name();
            if (error_type != "St16invalid_argument") std::cout << e.what() << std::endl; // ensure that error was from Date class
            else errors += 1; // ensure that error was from Date class
        }
    }
    if (errors == 1000) std::cout << "Test 1: SUCCESS" << std::endl;
    else throw std::runtime_error("Test 1 Failed: Invalid date did not trigger an exception");
}

void test2() {
    for (unsigned year = Date::first_year; year < Date::last_year; ++year) {
        for (unsigned month = 1; month <= 12; ++month) {
            unsigned max_day = days_in_month[month - 1];
            if (month == 2 && Date::is_leap_year(year)) max_day += 1; // Adjust for leap year
            for (unsigned day = 1; day <= max_day; ++day) {
                Date date(year, month, day);
                unsigned serial = date.get_serial();

                unsigned y, m, d;
                Date::serial_to_date(serial, y, m, d);

                if (year != y || month != m || day != d) {
                    std::cout << "serial is: " << serial << " " << y << "-" << m << "-" << d << std::endl;
                    std::cerr << "Test failed: Conversion mismatch for date " << year << "-" << month << "-" << day << std::endl;                    throw std::runtime_error("Date-to-serial and serial-to-date conversion failed");
                }
            }
        }
    }
    std::cout << "Test 2: SUCCESS" << std::endl;
}

void test3() {

    Date previous(1900, 1, 1);

    for (unsigned year = Date::first_year; year < Date::last_year; ++year) {
        for (unsigned month = 1; month <= 12; ++month) {
            unsigned max_day = days_in_month[month - 1];
            if (month == 2 && Date::is_leap_year(year)) max_day += 1; // Adjust for leap year
            for (unsigned day = 1; day <= max_day; ++day) {
                if (year==1900 && month==1 && day==1) continue;
                // std::cout << year << "-" << month << "-" << day << std::endl;
                Date current(year, month, day);
                // std::cout << "previous: " << previous.get_serial() << " current: " << current.get_serial() << std::endl;
                if (current.get_serial() != previous.get_serial() + 1) {
                    std::cerr << "Test failed: Serial numbers are not contiguous for dates " 
                                << previous.to_string() << " and " << current.to_string() << std::endl;
                    throw std::runtime_error("Contiguous date test failed");
                }
                previous = current;
            }
        }
    }

    std::cout << "Test 3: SUCCESS" << std::endl;
}

int main()
{
    test1();
    test2();
    test3();

    return 0;
}

