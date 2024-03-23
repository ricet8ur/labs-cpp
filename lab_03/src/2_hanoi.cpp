#include <iostream>
#include <cmath>
#include <sstream>
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
    if (argc != 2)
    {
        cout << "Wrong number of arguments\n";
        return 1;
    }
    int n;
    if (read_arg("first", argv[1], n))
        return 1;
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