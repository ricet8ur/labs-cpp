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

class base
{
protected:
    atomic<bool> _calculated{0};

public:
    ppp &context;
    size_t self;
    base(const base &other) : self{other.self}, context{other.context} {}
    base(ppp &context) : context{context}
    {
    }

    bool is_calculated() const
    {
        return _calculated.load();
    }
    virtual vector<size_t> get_next_exprs() = 0;
};

template <typename T>
class expression;

template <typename T>
class var final : public base
{
    T _result;
    prev
public:
    var(const var &other) : base{other} {}
    var(const expression &other) : base{other} {}

    T result()
    {
        return _result;
    }

    vector<size_t> get_next_exprs()
    {
        vector<size_t> new_calculators;
        if (!atomic_exchange(&_calculated, true))
        {
            _result = f();
            for (size_t next_expr : next_exprs)
            {
                // check next expression is ready to be calculated
                bool pre = true;
                for (size_t pre_expr : next_expr->prev_exprs)
                {
                    pre = pre && pre_expr->is_calculated();
                }
                if (pre && !next_expr->prev_exprs.empty())
                    new_calculators.push_back(next_expr);
            }
        }
        return new_calculators;
    }
};
// aka rvalue<T>
template <typename T>
class expression final : public base
{
    T _result;
    function<T()> f;

public:
    expression(const expression &other) : f{other.f}, base(other)
    {
        _calculated.store(other.is_calculated());
    }
    expression(ppp &context, function<T()> f) : f{f}, base{context}
    {
        _calculated.store(false);
        ++constext.aa;
    }
    expression(ppp &context, function<T()> f, T result) : _result{result}, f{f}, base(context)
    {
        _calculated.store(false);
        ++constext.aa;
    }
    T result()
    {
        return _result;
    }

    vector<size_t> get_next_exprs()
    {
        // cout << _result << 'a' << std::this_thread::get_id() << flush<<endl;
        vector<size_t> new_calculators;
        if (!atomic_exchange(&_calculated, true))
        {
            _result = f();
            // cout << _result << 'a' << std::this_thread::get_id() << flush<<endl;
            for (size_t next_expr : next_exprs)
            {
                // check next expression is ready to be calculated
                bool pre = true;
                for (size_t pre_expr : next_expr->prev_exprs)
                {
                    pre = pre && pre_expr->is_calculated();
                }
                if (pre && !next_expr->prev_exprs.empty())
                    new_calculators.push_back(next_expr);
            }
        }
        // cout << _result << 'g' << new_calculators.size() << endl;
        return new_calculators;
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
    exp.prev_exprs.emplace_back(lhs.self);
    exp.prev_exprs.emplace_back(rhs.self);
    lhs.next_exprs.emplace_back(exp.self);
    rhs.next_exprs.emplace_back(exp.self);
    cout << exp.context.q.size() << ' ';
    return exp;
}

class ppp
{
public:
    size_t aa = 0;
    unordered_map<size_t, base *> q;
    template <typename T>
    auto add_variable(const T t)
    {
        auto exp = expression(
            *this, function<T()>([t]() mutable -> T
                                 { 
                                    cout<<'p'<<endl;
                                return t; }),
            23.);
        q.emplace(aa, exp);
        ++aa;
        return exp;
    }

    auto calculate()
    {
        auto futures = deque<future<vector<size_t>>>{};
        while (!q.empty())
        {
            // start read-write parallel wrap
            for (auto [k, v] : q)
            {
                auto prom = promise<vector<size_t>>{};
                prom.set_value(v->get_next_exprs());
                futures.emplace_back(prom.get_future());
                // futures.emplace_back(async(&base::get_next_exprs, e.get()));
                cout << q.size() << 'b' << endl;
            }
            // cout << q.size() << 'r' << endl;

            // fill next wrap
            for (auto &fv : futures)
            {
                cout << q.size() << 'r' << endl;
                auto fw = fv.get();
                cout << q.size() << 'r' << endl;
                for (auto fa : fw)
                    q.emplace_back(fa);
            }
            futures.clear();
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