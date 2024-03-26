#include <iostream>
#include <chrono>
#include <cmath>
#include <sstream>
#include <limits>
using namespace std;
auto f1(double x)
{
    return pow(x, 2) - pow(x, 2) + x * 4 - x * 5 + x + x;
}

auto f2(double x)
{
    return x + x;
}

template <typename T, typename S1, typename S2>
bool read_arg(S1 name, S2 from, T &to)
{
    using namespace std;
    auto input = istringstream(from);
    cin.rdbuf(input.rdbuf());
    cin >> to;
    if (cin.fail() || !cin.eof())
    {
        cout << "Wrong " << name << " argument format\n";
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    using namespace std;
    if (argc != 3)
    {
        cout << "Wrong number of arguments\n";
        return 1;
    }
    char f_type{argv[1][0]};
    double x;
    if (argv[2] == string("inf"))
        x = std::numeric_limits<double>::infinity();
    else if (argv[2] == string("-inf"))
        x = -std::numeric_limits<double>::infinity();
    else if (argv[2] == string("nan"))
        x = std::numeric_limits<double>::quiet_NaN();
    else if (argv[2] == string("-nan"))
        x = -std::numeric_limits<double>::quiet_NaN();
    else if (read_arg("second", argv[2], x))
        return 1;
    if (f_type == '1')
        cout << f1(x);
    else if (f_type == '2')
        cout << f2(x);
    return 0;
}