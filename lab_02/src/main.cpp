#include <iostream>
#include <chrono>
#include <cmath>
#include <sstream>

double calculate(double x)
{
    return pow(x, 2) - pow(x, 2) + x * 4 - x * 5 + x + x;
}

// arg 1 = x
// arg 2 = n
int main(int argc, char *argv[])
{
    using namespace std;
    if (argc != 3)
    {
        cout << "Wrong number of arguments\n";
        return 1;
    }
    double x;
    int n;
    auto input = istringstream(argv[1]);
    cin.rdbuf(input.rdbuf());
    cin >> x;
    if (cin.fail() || !cin.eof())
    {
        cout << "Wrong first argument format\n";
        return 1;
    }
    input = istringstream(argv[2]);
    cin.rdbuf(input.rdbuf());
    cin >> n;
    if (cin.fail() || !cin.eof())
    {
        cout << "Wrong second argument format\n";
        return 1;
    }
    cout << "x = " << x << "\nn = " << n << endl;
    auto start = chrono::high_resolution_clock::now();
    for (int k = 0; k < n; ++k)
        x = calculate(x);
    auto end = chrono::high_resolution_clock::now();
    auto diff = end - start;
    cout << "x' = " << x << endl;
    cout << chrono::duration<double, milli>(diff).count() << " ms\n";
    return 0;
}