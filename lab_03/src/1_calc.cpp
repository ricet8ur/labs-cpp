#include <iostream>
#include <vector>
#include <charconv>

double my_pow(double a, int n)
{
    if (n == 0)
        return a;
    if (n > 0)
    {
        if (n % 2 == 0)
            return my_pow(a, n / 2);
        else
            return my_pow(a, n / 2) * a;
    }  
    return 1 / my_pow(a, -n);
}

int main(int argc, char *argv[])
{
    using namespace std;
    if (argc != 4)
    {
        cout << "Wrong number of arguments\n";
        return 1;
    }
    vector<string> v;
    for (size_t idx = 1; idx < argc; ++idx)
        v.push_back(string(argv[idx]));
    double a;
    double b;
    char op = v[1][0];
    auto [_ptr, ec] = from_chars(v[0].data(), v[0].data() + v[0].size(), a);
    if (ec != std::errc())
    {
        cout << "Wrong first argument format\n";
        return 1;
    }
    auto [_ptr2, ec2] = from_chars(v[2].data(), v[2].data() + v[2].size(), b);
    if (ec2 != std::errc())
    {
        cout << "Wrong second argument format\n";
        return 1;
    }
    double result;
    if (op == '+')
        result = a + b;
    else if (op == '-')
        result = a - b;
    else if (op == '*')
        result = a * b;
    else if (op == '^')
        result = my_pow(a, b);
    else
    {
        cout << "Wrong operation\n";
        return 1;
    }
    cout << a << " " << op << " " << b << " = " << result << endl;
    return 0;
}