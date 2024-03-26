#include <iostream>
#include <chrono>
#include <cmath>

double f1(double x)
{
    return pow(x, 2) - pow(x, 2) + x * 4 - x * 5 + x + x;
}

double f2(double x)
{
    return x + x;
}

void test(double x, size_t n)
{
    using namespace std;
    auto start = chrono::high_resolution_clock::now();
    for (size_t k = 0; k < n; ++k)
    {
        auto a = f1(x);
        auto b = f2(x);
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
    test(x, 10000);
    test(x, 100000);
    return 0;
}