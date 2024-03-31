#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <unordered_map>
#include <deque>
#include <functional>
#include <future>
#include <atomic>
using namespace std;

class ppp;
struct fwrapper
{
    function<vector<fwrapper>()> v;
};
using fvf = function<vector<fwrapper>()>;

template <typename T>
class expression
{
    atomic<bool> _calculated{0};
    T _result;
    function<T()> f;

public:
    ppp &context;
    vector<expression<T> *> next_exprs;
    vector<expression<T> *> prev_exprs;

    expression(const expression &other) : f{other.f}, context{other.context}, next_exprs{}
    {
        _calculated.store(other.is_calculated());
    }
    expression(ppp &context, function<T()> f) : f{f}, context{context}, next_exprs{} { _calculated.store(false); }
    expression(ppp &context, function<T()> f, T result) : _result{result}, f{f}, context{context}, next_exprs{} { _calculated.store(false); }

    bool is_calculated() const
    {
        return _calculated.load();
    }

    T result()
    {
        return _result;
    }

    fwrapper get_calculator()
    {
        return fwrapper
        {
            fvf
            {
                [&]() mutable -> vector<fwrapper>
                {
                if (is_calculated())
                    return vector<fwrapper>{};
                else{
                             _result = f();
                    cout<<_result<<'a'<<std::this_thread::get_id()<<endl;
                             _calculated.store(true);
                             return vector<fwrapper>{get_new_calculators()}; } }
            }
        };
    }

    fwrapper get_new_calculators()
    {
        return fwrapper{fvf{[&]() mutable -> vector<fwrapper>
                            {
            vector<fwrapper> new_calculators;
            for (expression<T> *next_expr : next_exprs)
            {
                // check next expression is ready to be calculated
                bool pre = true;
                for (expression<T> *pre_expr : next_expr->prev_exprs){
                    pre = pre && pre_expr->is_calculated();

                }
                if (pre&&!next_expr->prev_exprs.empty())
                    new_calculators.emplace_back(next_expr->get_calculator());

            }
                cout<<_result<<'g'<<new_calculators.size()<<endl;
            return new_calculators; }}};
    }
};
template <typename T>
auto operator+(expression<T> &lhs, expression<T> &rhs)
{
    auto exp = expression(
        lhs.context, function<T()>([&lhs, &rhs]() mutable -> T
                                   { 
                                    cout<<'d'<<endl;
                                    return lhs.result() + rhs.result(); }));
    exp.prev_exprs.emplace_back(&lhs);
    exp.prev_exprs.emplace_back(&rhs);
    lhs.next_exprs.emplace_back(&exp);
    rhs.next_exprs.emplace_back(&exp);
    cout << exp.context.q.size() << ' ';
    return exp;
}

class ppp
{

public:
    deque<fwrapper> q;
    // example T: double
    // example F: lambda(expression,expression)->T
    //            lambda()->T

    // example T: double
    template <typename T>
    auto add_variable(const T t)
    {
        auto exp = expression(
            *this, function<T()>([t]()mutable -> T
                            { 
                                    cout<<'p'<<endl;
                                return t; }),
            23.);
        exp.context.q.push_back(exp.get_calculator());
        return exp;
    }

    auto calculate()
    {
        auto futures = deque<future<vector<fwrapper>>>{};
        auto futures2 = deque<future<vector<fwrapper>>>{};
        while (!q.empty())
        {
            cout << q.size() << 'w' << endl;

            // start read-write parallel wrap
            for (auto &&c : q)
            {
                futures.emplace_back(async(c.v));
            }
            cout << q.size() << 'b' << endl;

            q.clear();
            // parallel read-only preparation wrap
            for (auto &fv : futures)
            {
                auto v = fv.get();
                cout << q.size() << v.size() << 'c' << endl;
                for (auto &fw : v)
                {
                    futures2.emplace_back(async(fw.v));
                }
            }
            futures.clear();

            // fill next wrap
            for (auto &&fv : futures2)
            {
                for (auto &&fw : fv.get())
                    q.emplace_back(fw);
            }
            futures2.clear();
        }
    }
};
// void test(double x, size_t n)
// {
//     using namespace std;
//     auto start = chrono::high_resolution_clock::now();
//     for (size_t k = 0; k < n; ++k)
//     {
//         auto at = thread(f1, x);
//         auto bt = thread(f2, x);
//         at.join();
//         bt.join();
//         x = a + b - a;
//     }
//     auto end = chrono::high_resolution_clock::now();
//     auto diff = end - start;
//     cout << chrono::duration<double, chrono::seconds::period>(diff).count() << " s\n";
//     cout << "x' = " << x << "\n";
// }

int main(int argc, char *argv[])
{
    using namespace std;
    double x = 1;
    ppp p;
    auto xp = p.add_variable(x);
    auto yp = p.add_variable(2.);
    auto w = xp + yp;
    p.calculate();
    return 0;
}