#include <iostream>
#include <cmath>
#include <vector>
#include <charconv>
#include <deque>
#include <utility>
using namespace std;

int third(int from, int to)
{
    return 3 - from - to;
}

void calculate(int from, int to, int x, deque<pair<int, int>> &g)
{
    if (x <= 0)
        return;
    calculate(from, third(from, to), x - 1, g);
    g.push_back({from, to});
    calculate(third(from, to), to, x - 1, g);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Wrong number of arguments\n";
        return 1;
    }
    vector<string> v;
    for (size_t idx = 1; idx < argc; ++idx)
        v.push_back(string(argv[idx]));
    int n;
    auto [_ptr, ec] = from_chars(v[0].data(), v[0].data() + v[0].size(), n);
    if (ec != std::errc())
    {
        cout << "Wrong first argument format\n";
        return 1;
    }
    deque<pair<int, int>> g;
    calculate(0, 2, n, g);
    while (g.size() > 0)
    {
        auto [a, b] = g.front();
        g.pop_front();
        cout << a << b << ' ';
    }
    return 0;
}