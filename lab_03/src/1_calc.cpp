#include <iostream>
#include <sstream>

double my_pow(double a, int n)
{
    if (n == 0)
        return 1;
    if (n > 0)
    {
        double part = my_pow(a, int(n / 2));
        if (n % 2 == 0)
            return part * part;
        else
            return part * part * a;
    }
    return 1 / my_pow(a, -n);
}

// works for 0<x<2
double log_teylor_series(double x, size_t N = 5)
{
    const auto a = x - 1;
    auto a_k = a;
    auto y = a; // log value
    for (size_t k = 2; k < N; ++k)
    {
        a_k = -a_k * a;
        y = y + a_k / k;
    }
    return y;
}

double my_sqrt(double d)
{
    double root = d / 2;
    double eps = 1e-14;
    for (size_t k = 0; k < 100; ++k)
    {
        if (root * root - d < eps * root && root * root - d > -eps * root)
            break;
        root = 0.5 * (root + d / root);
    }
    return root;
}

double ln_reduction(double x, size_t N)
{
    const auto b = 0.3;
    int n = 0;
    auto t = x;
    while (1 + b <= t)
    {
        n += 1;
        t = my_sqrt(t);
    }
    return my_pow(2, n) * log_teylor_series(t, N = N);
}

double my_ln(double x, double eps)
{
    const auto b = 0.3;
    int n = 1;
    const auto a = x - 1;
    if (a > 0)
    {

        while (my_pow(b / (1 + b), int(n + 1)) / (n + 1) > eps)
        {
            n += 1;
            if (n > 100)
                break;
        }
    }
    else
        return -my_ln(1 / x, eps);
    return ln_reduction(x, n);
}

// works for 0<x<2, arbitrary r
double exp_teylor_series(double r, size_t N = 5)
{
    const auto a = r;
    auto a_k = 1.;
    auto y = 1.; // exp value
    for (size_t k = 1; k < N; ++k)
    {
        a_k *= a / k;
        y += a_k;
    }
    return y;
}

double my_pow(double a, double b)
{
    if (a < 0)
        a = -a;
    if (a == 0.)
        return b == 0. ? 1 : 0;
    // a^b = exp(b ln a)
    // exp(x) = exp^(z ln1.5 + r)
    // exp(x) = 1.5^(z) exp(r)
    // ln 1.5 since teylor works for x < 2
    auto eps = 1e-10;
    auto x = b * my_ln(a, eps);
    auto c = log_teylor_series(1.5, 100);
    auto z = int(x / c);
    auto r = x - z * c;

    return my_pow(1.5, int(z)) * exp_teylor_series(r, 100);
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
    // for debug purposes
    // argc = 4;
    // char* arg[] = {"2","1e99","^","-1e-100"};
    // argv = arg;
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
    {
        if (double(int(b)) == b)
        {
            result = my_pow(a, int(b));
            cout << "b is an integer: " << b << '\n';
        }
        else
        {
            result = my_pow(a, b);
            cout << "b is double: " << b << '\n';
        }
    }
    else
    {
        cout << "Wrong operation \'" << op << "\'. Use only: + - * ^\n";
        return 1;
    }
    cout << a << " " << op << " " << b << " = " << result << endl;
    return 0;
}