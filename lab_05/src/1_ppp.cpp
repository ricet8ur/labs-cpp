#include <atomic>
#include <chrono>
#include <cmath>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class ppp {
	class base {
	protected:
		atomic<bool> _calculated { false };
		bool rval { false };

	public:
		atomic<bool> _to_be_calculated { false };
		size_t self;
		vector<size_t> next;
		vector<size_t> prev;
		base(size_t id)
			: self { id }
			, next {}
			, prev {}
		{
		}
		base()
			: self {}
			, next {}
			, prev {}
		{
		}
		base(const base& other)
			: self { other.self }
			, next { other.next }
			, prev { other.prev }
		{
		}

		bool is_calculated() const
		{
			return _calculated.load();
		}
		virtual vector<size_t> get_next_exprs() = 0;
		virtual ~base()
		{
		}
	};

public:
	size_t aa = 0;
	unordered_map<size_t, base*> exps;
	unordered_set<size_t> vars;

	auto calculate()
	{
		deque<size_t> q;
		for (auto var : vars) {
			if (exps.at(var)->is_calculated())
				q.push_back(var);
		}
		auto futures = deque<future<vector<size_t>>> {};
		while (!q.empty()) {
			// start read-write parallel wrap
			for (auto id : q) {
				auto* v = exps.at(id);
				// auto prom = promise<vector<size_t>>{};
				// prom.set_value(v->get_next_exprs());
				// futures.emplace_back(prom.get_future());
				futures.emplace_back(async(&base::get_next_exprs, v));
				// cout << q.size() << 'b' << endl;
			}
			// cout << q.size() << 'r' << endl;
			q.clear();
			// fill next wrap
			for (auto& fv : futures) {
				// cout << q.size() << 'r' << endl;
				auto vec_id = fv.get();
				// cout << q.size() << 'r' << endl;
				for (auto id : vec_id)
					q.push_back(id);
			}
			futures.clear();
		}
	}

	template <typename T>
	class expression final : public base {
		T _result;
		function<T(T, T)> f;

	public:
		ppp& context;
		expression(ppp& context, function<T(T, T)> f)
			: base {}
			, f { f }
			, context { context }
		{
			_calculated.store(false);
			self = ++context.aa;
			context.exps.insert(make_pair(self, this));
		}
		expression(ppp& context, function<T(T, T)> f, T result)
			: base {}
			, _result { result }
			, f { f }
			, context { context }
		{
			_calculated.store(true);
			self = ++context.aa;

			context.exps.insert(make_pair(self, this));
		}
		~expression()
		{
		}
		T result()
		{
			return _result;
		}

		vector<size_t> get_next_exprs()
		{
			vector<size_t> new_calculators;
			if (!atomic_exchange(&_calculated, true)) {
				auto ra = static_cast<expression*>(context.exps.at(prev[0]))->result();
				auto rb = static_cast<expression*>(context.exps.at(prev[1]))->result();
				_result = f(ra, rb);
			}
			for (size_t next_id : next) {
				// check if next expression is ready to be calculated
				bool ready = true;
				auto* next_ex = context.exps.at(next_id);
				auto nextprev = next_ex->prev;
				for (size_t pre_id : nextprev)
					ready = ready && context.exps.at(pre_id)->is_calculated();
				if (ready && !atomic_exchange(&next_ex->_to_be_calculated, true))
					new_calculators.push_back(next_id);
			}
			return new_calculators;
		}

		void bind(expression& rhs, expression& lhs, expression& res)
		{
			context.exps.at(res.self)->prev.emplace_back(lhs.self);
			context.exps.at(res.self)->prev.emplace_back(rhs.self);
			context.exps.at(lhs.self)->next.emplace_back(res.self);
			context.exps.at(rhs.self)->next.emplace_back(res.self);
		}

		auto operator+(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra + rb;
					ostringstream ss;
					ss << ra << '+' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
					return res;
				}));
			bind(*this, rhs, *exp);
			return *exp;
		}

		auto operator-(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra - rb;
					ostringstream ss;
					ss << ra << '-' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
					return res;
				}));
			bind(*this, rhs, *exp);
			return *exp;
		}

		auto operator*(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra * rb;
					ostringstream ss;
					ss << ra << '*' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
					return res;
				}));
			bind(*this, rhs, *exp);
			return *exp;
		}

		auto operator/(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra / rb;
					ostringstream ss;
					ss << ra << '/' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
					return res;
				}));
			bind(*this, rhs, *exp);
			return *exp;
		}

		auto operator+(expression&& rhs)
		{
			return *this + rhs;
		}
		auto operator-(expression&& rhs)
		{
			return *this - rhs;
		}
		auto operator*(expression&& rhs)
		{
			return *this * rhs;
		}
		auto operator/(expression&& rhs)
		{
			return *this / rhs;
		}
	};

	template <typename T>
	expression<T> add_variable(const T t)
	{
		auto exp = expression(*this, function<T(T, T)>([](T ra, T rb) -> T {
			cout << "never call" << endl;
			return ra;
		}),
			t);
		vars.insert(exp.self);
		return exp;
	}
	template <typename T>
	expression<T> add_variable(const expression<T> exp)
	{
		vars.insert(exp.self);
		return exp;
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
//     cout << chrono::duration<double, chrono::seconds::period>(diff).count()
//     << " s\n"; cout << "x' = " << x << "\n";
// }

int main(int argc, char* argv[])
{
	using namespace std;
	double x = 1;
	ppp p;
	auto xp = p.add_variable(x);
	auto yp = p.add_variable(2.);
	auto w = (xp + yp) + (yp + yp);
	p.calculate();
	// cout << p.exps.size() << endl;
	// cout << static_cast<ppp::expression<double> *>(p.exps.at(4))->result()
	// << endl;
	return 0;
}