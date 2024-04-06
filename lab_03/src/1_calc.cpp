#include <iostream>
#include <sstream>
#include <cassert>
#include <limits>



// precision bound
constexpr double eps = 1e-14;

double my_div(double a, double b){
    if(b==0.||b==-0.){
        if (a==0.||a==-0.){
            return std::numeric_limits<double>::quiet_NaN(); // since no convergence
        }
        return a*(b==0.?1:-1)*std::numeric_limits<double>::infinity();
    }
    auto negative = (a>0.)^(b>0.);
    // find inverse of b
    a = a>0.? a:-a;
    b = b>0.? b:-b;
    auto pa = 0.;
    auto pb = std::numeric_limits<double>::max();
    double c;
    while(pb-pa>eps*pa){
        c =pa+ (pb-pa)*0.5;
        if(c*b<1.)
            pa=c;
        else
            pb=c;
    }
    return negative? -a*c:a*c; 
}


int my_div(int a, int b){
    return int(my_div(double(a),double(b)));
}

// pow(double, int)
double constexpr my_pow(double a, int n)
{
    // by def
    if (n == 0)
        return 1;
    // border case
    if (a == 0.)
        return n > 0 ? 0 : std::numeric_limits<double>::infinity();

    if (n > 0)
    {
        auto part = my_pow(a, my_div(n,2));
        if (n % 2 == 0)
            return part * part;
        else
            return part * part * a;
    }
    else
        return my_div(1., my_pow(a, -n));
}

// pow(double, double)

// iteration bound
constexpr size_t K = 100;
// reduction bound
constexpr double B = 0.3;
// Finding necessary number of iterations for taylor serier of ln(x)
// knowing the bound B of an argument due to reduction
// error of ln(1+x) approximation is given by the first discarded term
// | B^n/n |
size_t constexpr ln_iterations_count()
{
    int n = 1;
    while (my_div(my_pow(B, n + 1), (n + 1.)) > eps)
    {
        ++n;
        if (n > K)
            break;
    }
    return n;
}

double ln_taylor(double x)
{
    assert((0. < x) && (x < 2.) && "x out of range (0,2) when calling ln_taylor(x)");
    if (x == 0.)
        return -std::numeric_limits<double>::infinity();
    if (x < 0. || x >= 2.)
        return std::numeric_limits<double>::quiet_NaN(); // since no convergence

    x = x - 1;
    auto a_k = x;
    auto y = x; // log value
    for (size_t k = 2; k < ln_iterations_count(); ++k)
    {
        a_k = -a_k * x;
        y += my_div(a_k, double(k));
    }
    return y;
}

double my_sqrt(double x)
{
    assert((x >= 0.) && "x < 0 when calling my_sqrt(x)");

    double r = my_div(x, 2.); // root
    for (size_t k = 0; k < K; ++k)
    {
        if (r * r - x < eps && r * r - x > -eps)
            break;
        r = my_div((r + my_div(x, r)),  2.);
    }
    return r;
}

double my_ln(double x)
{
    assert((x > 0.) && "x <= 0 when calling my_ln(x)");
    if (x < 0.)
        return std::numeric_limits<double>::quiet_NaN();
    if (x < 1)
        return -my_ln(my_div(1., x));

    // reduction
    int n = 0;
    while (1 + B <= x)
    {
        n += 1;
        x = my_sqrt(x);
    }
    return my_pow(2, n) * ln_taylor(x);
}

size_t constexpr exp_iterations_count()
{
    int n = 1;
    // B^n decreases fast enough
    // to not take n! into consideration
    while (my_pow(B, n + 1) > eps)
    {
        ++n;
        if (n > K)
            break;
    }
    return n;
}

// expect x in [-1,1] for faster convergence
double exp_taylor(double x)
{
    assert((-1. < x) && (x < 1.) && "x out of range (-1,1) when calling exp_taylor(x)");
    if (x == 0.)
        return 1.;

    auto a_k = 1.;
    auto y = 1.; // exp value
    for (size_t k = 1; k < exp_iterations_count(); ++k)
    {
        a_k *= my_div(x, k);
        y += a_k;
    }
    return y;
}

// taylor + argument reduction
double my_pow(double a, double b)
{
    // by def
    if (a < 0.)
        a = -a;
    if (double(int(b)) == b)
        return my_pow(a, int(b));
    if (a == 0.)
        return b == 0. ? 1 : 0;
    // a > 0.
    // a^b = exp(b ln a)
    // exp(x) = exp^(z ln2 + r)
    //        = 2^z exp(r)
    auto x = b * my_ln(a);
    auto c = my_ln(2);
    auto z = int(my_div(x, c));
    auto r = x - z * c;

    return my_pow(2, z) * exp_taylor(r);
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
        result = my_pow(a, b);
    else
    {
        cout << "Wrong operation \'" << op << "\'. Use only: + - * ^\n";
        return 1;
    }
    cout << a << " " << op << " " << b << " = " << result << endl;
    return 0;
}