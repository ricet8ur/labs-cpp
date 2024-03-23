#include <iostream>
#include <sstream>

double my_pow(double a, int n)
{
    if (n == 0)
        return 1;
    if (n > 0)
    {
        double part = my_pow(a, n / 2);
        if (n % 2 == 0)
            return part * part;
        else
            return part * part * a;
    }
    return 1 / my_pow(a, -n);
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
    if (argc != 4)
    {
        cout << "Wrong number of arguments\n";
        return 1;
    }
    double a;
    double b;
    char op = argv[2][0];
    if (read_arg("first", argv[1], a))
        return 1;
    if (read_arg("third", argv[3], b))
        return 1;
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
        cout << "Wrong operation \'" << op << "\'. Use only: + - * ^\n";
        return 1;
    }
    cout << a << " " << op << " " << b << " = " << result << endl;
    return 0;
}