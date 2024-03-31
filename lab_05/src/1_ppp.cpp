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
    shared_ptr<base> self;
    vector<shared_ptr<base>> next_exprs;
    vector<shared_ptr<base>> prev_exprs;
    base(shared_ptr<base> other):self{other}{}
    bool is_calculated() const
    {
        return _calculated.load();
    }
    // virtual void calculate() = 0;
    virtual vector<shared_ptr<base>> get_next_exprs() = 0;
};

template <typename T>
class expression : public base
{
    T _result;
    function<T()> f;

public:
    ppp &context;
    expression(const expression &other) : f{other.f}, context{other.context}, base(shared_ptr<expression>(this))
    {
        _calculated.store(other.is_calculated());
    }
    // expression(const expression *other) : f{other->f}, context{other->context}, base(shared_ptr<expression>(this))
    // {
    //     _calculated.store(other->is_calculated());
    // }
    expression(ppp &context, function<T()> f) : f{f}, context{context}, base(shared_ptr<expression>(this)) { _calculated.store(false); }
    expression(ppp &context, function<T()> f, T result) : _result{result}, f{f}, context{context}, base(shared_ptr<expression>(this)) { _calculated.store(false); }

    T result()
    {
        return _result;
    }

    // void calculate()
    // {
    //     if (!atomic_exchange(&_calculated, true))
    //     {
    //         _result = f();
    //         cout << _result << 'a' << std::this_thread::get_id() << endl;
    //     }
    // }

    vector<shared_ptr<base>> get_next_exprs()
    {
            // cout << _result << 'a' << std::this_thread::get_id() << flush<<endl;
        vector<shared_ptr<base>> new_calculators;
        if (!atomic_exchange(&_calculated, true))
        {
            _result = f();
            // cout << _result << 'a' << std::this_thread::get_id() << flush<<endl;
            for (shared_ptr<base> next_expr : next_exprs)
            {
                // check next expression is ready to be calculated
                bool pre = true;
                for (shared_ptr<base> pre_expr : next_expr->prev_exprs)
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
    deque<shared_ptr<base>> q;
    // example T: double
    // example F: lambda(expression,expression)->T
    //            lambda()->T

    // example T: double
    template <typename T>
    auto add_variable(const T t)
    {
        auto exp = expression(
            *this, function<T()>([t]() mutable -> T
                                 { 
                                    cout<<'p'<<endl;
                                return t; }),
            23.);
        q.emplace_back(exp.self);
        // exp.context.q.push_back(exp);
        return exp;
    }

    auto calculate()
    {
        // auto futures = deque<pair<shared_ptr<base>, future<void>>>{};
        auto futures = deque<future<vector<shared_ptr<base>>>>{};
        while (!q.empty())
        {
            cout << q.size() << 'w' << endl;

            // start read-write parallel wrap
            for (auto e : q)
            {
                futures.emplace_back(async(&base::get_next_exprs, e));
                cout << q.size() << 'b' << endl;
            }
            cout << q.size() << 'r' << endl;

            // // parallel read-only preparation wrap
            // for (auto &ef : futures)
            // {
            //     auto &[e, f] = ef;
            //     cout << futures.size() << 'c' << endl;
            //     f.get();
            //     // e->get_next_exprs();
            //     futures2.emplace_back(async(&base::get_next_exprs, e));
            // }
             q.clear();

            // fill next wrap
            for (auto &fv : futures)
            {
                cout << q.size() << 'r' << endl;
                auto fw = fv.get();
                cout << q.size() << 'r' << endl;
                for (auto fw : fv.get())
                    q.emplace_back(fw);
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