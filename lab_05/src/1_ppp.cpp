#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
using namespace std;

#define PRINT_OPERATIONS_OF_LAMBDAS

class ppp {
public:
	bool print_thread_info { false };
	chrono::system_clock::time_point time_ppp_start { chrono::high_resolution_clock::now() };
	class base {
	protected:
		bool _calculated { false };
		bool rval { false };

	public:
		size_t self;
		vector<size_t> next;
		vector<size_t> prev;
		base()
			: self {}
			, next {}
			, prev {}
		{
		}

		bool is_calculated() const
		{
			return _calculated;
		}
		virtual void copy_result_from(base* other) = 0;
		virtual base* deep_copy() = 0;
		virtual vector<size_t> get_next_exprs() = 0;
		virtual ~base()
		{
		}
	};

	size_t aa = 0;
	unordered_map<size_t, base*> exps;
	unordered_set<size_t> vars;

	auto calculate()
	{
		vector<size_t> q;
		for (auto var : vars) {
			if (exps.at(var)->is_calculated())
				q.emplace_back(var);
		}

		auto futures = vector<future<vector<size_t>>> {};
		while (!q.empty()) {
			// start read-write parallel wrap
			for (auto id : q) {

				auto* v = exps.at(id);
				futures.emplace_back(async(&base::get_next_exprs, v));
			}
			q.clear();
			// fill next wrap
			for (auto& fv : futures) {
				// cout << exps.size() << endl;
				auto vec_id = fv.get();
				// cout << exps.size() << endl;

				for (auto id : vec_id)
					q.push_back(id);
			}

			futures.clear();
			sort(q.begin(), q.end());
			auto last = std::unique(q.begin(), q.end());
			q.erase(last, q.end());
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
			_calculated = false;
			self = ++context.aa;
			context.exps.emplace(self, this);
		}
		expression(ppp& context, function<T(T, T)> f, T result)
			: base {}
			, _result { result }
			, f { f }
			, context { context }
		{
			_calculated = true;
			self = ++context.aa;

			context.exps.emplace(self, this);
		}
		~expression()
		{
		}
		T result()
		{
			return _result;
		}
		void copy_result_from(base* other) override
		{
			_result = static_cast<expression*>(other)->result();
		}

		vector<size_t> get_next_exprs() override
		{
			auto start = chrono::high_resolution_clock::now();
			vector<size_t> new_calculators;
			if (!_calculated) {
				_calculated = true;
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
				if (ready)
					new_calculators.push_back(next_id);
			}
			if (context.print_thread_info) {
				ostringstream ss;
				ss.setf(ios::fixed);
				ss << setprecision(0);
				ss << "expression_id: " << self << " | thread_id: " << this_thread::get_id();

				auto end = chrono::high_resolution_clock::now();
				auto diff1 = start - context.time_ppp_start;
				auto diff2 = end - context.time_ppp_start;
				auto diff3 = end - start;
				ss << " | start: " << chrono::duration<double, chrono::microseconds::period>(diff1).count() << " us";
				ss << " | end: " << chrono::duration<double, chrono::microseconds::period>(diff2).count() << " us";
				ss << " | delta: " << chrono::duration<double, chrono::microseconds::period>(diff3).count() << " us\n";
				cout << ss.str() << flush;
			}
			return new_calculators;
		}

		void bind_expr(expression& rhs, expression& lhs, expression& res)
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
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
					ostringstream ss;
					ss << ra << '+' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
#endif
					return res;
				}));
			bind_expr(*this, rhs, *exp);
			return *exp;
		}

		auto operator-(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra - rb;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
					ostringstream ss;
					ss << ra << '-' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
#endif
					return res;
				}));
			bind_expr(*this, rhs, *exp);
			return *exp;
		}

		auto operator*(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra * rb;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
					ostringstream ss;
					ss << ra << '*' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
#endif
					return res;
				}));
			bind_expr(*this, rhs, *exp);
			return *exp;
		}

		auto operator/(expression& rhs)
		{
			auto* exp = new expression(context,
				function<T(T, T)>([](T ra, T rb) {
					const auto res = ra / rb;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
					ostringstream ss;
					ss << ra << '/' << rb << '=' << res << '\n';
					cout << ss.str() << flush;
#endif
					return res;
				}));
			bind_expr(*this, rhs, *exp);
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

		base* deep_copy() override
		{
			auto* exp = new expression(*this);
			return exp;
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

	template <typename T>
	void for_loop(T a, T b, T c, function<void(ppp&, T)> g)
	{
		// deep copy of state to support a new scope
		ppp pre_scope;
		pre_scope.aa = aa;
		pre_scope.print_thread_info = print_thread_info;
		pre_scope.vars.insert(vars.begin(), vars.end());
		for (auto [k, v] : exps) {
			pre_scope.exps.emplace(k, v->deep_copy());
		}
		while (a < b) {
			g(*this, a);
			calculate();

			a += c;
			// revert state back to pre to support a new scope
			// update state of exps
			for (auto [k, v] : pre_scope.exps) {
				v->copy_result_from(exps.at(k));
			}
			// revert to initial state
			for (auto [k, v] : pre_scope.exps) {
				*exps.at(k) = *v;
			}
			decltype(exps) tmp;
			for (auto [k, v] : exps) {
				if (pre_scope.exps.find(k) != pre_scope.exps.end()) {
					tmp.emplace(k, v);
				} else {
					delete v;
				}
			}
			vars = pre_scope.vars;
			aa = pre_scope.aa;
			print_thread_info = pre_scope.print_thread_info;
		}
	}
};

int main(int argc, char* argv[])
{
	auto start = chrono::high_resolution_clock::now();
	double x = 1;
	ppp p;
	p.print_thread_info = true;
	auto xp = p.add_variable(x);
	auto yp = p.add_variable(2.);
	p.for_loop(1, 5, 1,
		function<void(ppp&, int)>(
			[&](ppp& pp, int k) {
				auto w = (xp + yp) + (yp + yp);
			}));
	p.calculate();
	// cout << p.exps.size() << endl;
	// cout << static_cast<ppp::expression<double> *>(p.exps.at(4))->result()<< endl;
	auto end = chrono::high_resolution_clock::now();
	auto diff = end - start;
	cout << chrono::duration<double, chrono::milliseconds::period>(diff).count() << " ms - total execution time" << endl;
	return 0;
}