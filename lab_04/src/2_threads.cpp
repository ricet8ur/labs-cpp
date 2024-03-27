#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>

double a, b;

void f1(double x)
{
    a = pow(x, 2) - pow(x, 2) + x * 4 - x * 5 + x + x;
}

void f2(double x)
{
    b = x + x;
}

void test(double x, size_t n)
{
    using namespace std;
    auto start = chrono::high_resolution_clock::now();
    for (size_t k = 0; k < n; ++k)
    {
        auto at = thread(f1, x);
        auto bt = thread(f2, x);
        at.join();
        bt.join();
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