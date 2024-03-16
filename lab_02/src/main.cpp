#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <charconv>

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
    vector<string> v;
    for (size_t idx = 1; idx < argc; ++idx)
        v.push_back(string(argv[idx]));
    double x;
    int n;
    auto [_ptr, ec] = from_chars(v[0].data(), v[0].data()+v[0].size(),x);   
    if (ec != std::errc()){
        cout << "Wrong first argument format\n";
        return 1;
    }
    auto [_ptr2, ec2] = from_chars(v[1].data(), v[1].data()+v[1].size(),n);
    if (ec2 != std::errc()){
        cout << "Wrong second argument format\n";
        return 1;
    }
    cout << "x = " << x<<"\nn = " << n << endl;
    auto start = chrono::steady_clock::now();
    for (int k = 0; k < n; ++k)
        x = pow(x, 2) - pow(x, 2) + x * 4 - x * 5 + x + x;
    auto end = chrono::steady_clock::now();
    auto diff = end - start;
    cout << "x' = " << x << endl;
    cout << chrono::duration<double, milli>(diff).count() << " ms\n";
    return 0;
}