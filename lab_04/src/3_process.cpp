#include <iostream>
#include <chrono>
#include <cmath>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <charconv>
#include <limits>
#include <sstream>

namespace bp = boost::process;
namespace ba = boost::asio;
using namespace std;

template <unsigned int f_type>
future<string> compute(double x)
{
    ba::io_service ios;
    future<string> data;
    bp::child c("../build/3_aux_f", to_string(f_type), to_string(x),
                bp::std_in.close(),
                bp::std_err.close(),
                bp::std_out > data, // so it can be written without anything
                ios);
    ios.run();
    return data;
}

void test(double x, size_t n)
{
    auto start = chrono::high_resolution_clock::now();
    for (size_t k = 0; k < n; ++k)
    {
        auto fa = compute<1>(x);
        auto fb = compute<2>(x);
        // this will block
        auto sa = fa.get();
        auto sb = fb.get();
        double a;
        double b;
        auto read_chars = [](string s, double &x)
        {
            auto [ptr, ec] = from_chars(s.data(), s.data() + s.size(), x);
            if (ec != std::errc())
            {
                // inf and -inf cases
                if (s == string("inf"))
                    x = std::numeric_limits<double>::infinity();
                else if (s == string("-inf"))
                    x = -std::numeric_limits<double>::infinity();
                else if (s == string("nan"))
                    x = std::numeric_limits<double>::quiet_NaN();
                else if (s == string("-nan"))
                    x = -std::numeric_limits<double>::quiet_NaN();
                else
                    throw std::invalid_argument("string parsing error");
            }
        };
        read_chars(sa, a);
        read_chars(sb, b);
        x = a + b - a;
    }
    auto end = chrono::high_resolution_clock::now();
    auto diff = end - start;
    cout << chrono::duration<double, chrono::seconds::period>(diff).count() << " s\n";
    cout << "x' = " << x << "\n";
}

int main(int argc, char *argv[])
{
    using namespace std;
    double x = 1;
    test(x, 100);
    test(x, 1000);
    return 0;
}