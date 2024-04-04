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
#include <variant>
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
		virtual vector<size_t> execute() = 0;
		virtual ~base()
		{
		}
	};

	size_t aa = 0;
	unordered_map<size_t, base*> exps;
	unordered_set<size_t> vars;
	unordered_map<size_t, size_t> overwrite_assignment_storage;
	unordered_map<size_t, vector<variant<string, size_t>>> cout_bind_to_expression;
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
				futures.emplace_back(async(&base::execute, v));
			}
			q.clear();
			// fill next wrap
			for (auto& fv : futures) {
				auto vec_id = fv.get();
				for (auto id : vec_id)
					q.push_back(id);
			}

			futures.clear();
			sort(q.begin(), q.end());
			auto last = unique(q.begin(), q.end());
			q.erase(last, q.end());
		}
	}

	template <typename T>
	using ftype = function<T(ppp&, size_t)>;

	template <typename T>
	class expression final : public base {
		T _result;
		ftype<T> f;

	public:
		ppp& context;
		expression(ppp& context, ftype<T> f)
			: base {}
			, f { f }
			, context { context }
		{
			_calculated = false;
			self = ++context.aa;
			context.exps.emplace(self, this);
		}
		expression(ppp& context, ftype<T> f, T result)
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
		T result() const
		{
			return _result;
		}
		void copy_result_from(base* other) override
		{
			_result = static_cast<expression*>(other)->result();
		}

		vector<size_t> execute() override
		{
			auto start = chrono::high_resolution_clock::now();
			vector<size_t> new_calculators;
			if (!_calculated) {
				_calculated = true;
				_result = f(context, self);
			}
			for (size_t next_id : next) {
				// check if next expression is ready to be calculated
				bool ready = true;
				const auto* next_ex = context.exps.at(next_id);
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

		static void bind_expr(const expression& from, const expression& to)
		{
			from.context.exps.at(to.self)->prev.emplace_back(from.self);
			from.context.exps.at(from.self)->next.emplace_back(to.self);
		}
		static void bind_expr2(const expression& rhs, const expression& lhs, const expression& res)
		{
			bind_expr(lhs, res);
			bind_expr(rhs, res);
		}

		expression operator=(const expression& other)
		{
			if (other.self == self)
				return *this;
			auto* exp = new expression(context,
				ftype<T>([=](ppp& p, size_t id) {
					return p.exps.at(id)->prev[0];
				}));
			bind_expr(other, *exp);
			if (find(other.prev.begin(), other.prev.end(), self) != other.prev.end()) {
				context.overwrite_assignment_storage.emplace(self, other.self);
			}
			return *exp;
		}

		auto* generic_binary_operator(expression& rhs, function<T(T, T)> f)
		{
			auto* exp = new expression(context,
				ftype<T>([=](ppp& p, size_t id) {
					const auto& prev = p.exps.at(id)->prev;
					const auto ra = static_cast<expression*>(p.exps.at(prev[0]))->result();
					const auto rb = static_cast<expression*>(p.exps.at(prev[1]))->result();
					const auto res = f(ra, rb);
					return res;
				}));
			bind_expr2(*this, rhs, *exp);
			return exp;
		}

		auto operator+(expression& rhs)
		{
			return *generic_binary_operator(rhs, [](T a, T b) {
				const auto res = a + b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '+' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		auto operator-(expression& rhs)
		{
			return *generic_binary_operator(rhs, [](T a, T b) {
				const auto res = a - b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '-' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		auto operator*(expression& rhs)
		{
			return *generic_binary_operator(rhs, [](T a, T b) {
				const auto res = a * b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '*' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		auto operator/(expression& rhs)
		{
			return *generic_binary_operator(rhs, [](T a, T b) {
				const auto res = a / b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '/' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		auto
		operator+(expression&& rhs)
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
			return static_cast<base*>(exp);
		}
	};

	template <typename T>
	expression<T> add_variable(const T t)
	{
		auto* exp = new expression(*this, ftype<T>([](ppp& p, size_t id) {
			cout << "never call" << endl;
			return T {};
		}),
			t);
		vars.insert(exp->self);
		return *exp;
	}
	template <typename T>
	expression<T> add_variable(const expression<T>& other)
	{
		auto* exp = new expression(*this, ftype<T>([](ppp& p, size_t id) {
			return p.exps.at(id)->prev.at(0);
		}));
		vars.insert(exp->self);
		exp->bind_expr(other, *exp);
		return *exp;
	}

	template <typename T>
	void for_loop(T a, T b, T c, function<void(ppp&, T)> g)
	{
		calculate();
		// deep copy of state to support a new scope
		ppp pre_scope;
		pre_scope.aa = aa;
		pre_scope.print_thread_info = print_thread_info;
		pre_scope.overwrite_assignment_storage = overwrite_assignment_storage;
		pre_scope.vars.insert(vars.begin(), vars.end());
		for (auto [k, v] : exps) {
			pre_scope.exps.emplace(k, v->deep_copy());
		}
		while (a < b) {
			g(*this, a);
			// calculate();

			a += c;
			// to support a new scope
			// revert state back to pre
			for (auto [k, v] : pre_scope.exps) {
				// get results from inner scope
				// take overwrite_assignment_storage into account
				auto it = overwrite_assignment_storage.find(k);
				if (it != overwrite_assignment_storage.end()) {
					// find last overwrite of id k
					while (overwrite_assignment_storage.find(it->second) != overwrite_assignment_storage.end())
						it = overwrite_assignment_storage.find(it->second);
					// pull result into outer scope
					v->copy_result_from(exps.at(it->second));
				} else {
					v->copy_result_from(exps.at(k));
				}
			}
			// revert global state to initial state partially (only inner expression states)
			for (auto [k, v] : pre_scope.exps) {
				*exps.at(k) = *v;
				exps.at(k)->copy_result_from(v);
			}
			// revert global state to initial state completely (remove newly added expressions)
			decltype(exps) tmp;
			for (auto [k, v] : exps) {
				if (pre_scope.exps.find(k) != pre_scope.exps.end()) {
					tmp.emplace(k, v);
				} else if (vars.find(k) == vars.end()) {
					// since vars, created inside loop are already deleted
					delete v;
				}
			}
			exps = tmp;
			vars = pre_scope.vars;
			aa = pre_scope.aa;
			print_thread_info = pre_scope.print_thread_info;
			overwrite_assignment_storage = pre_scope.overwrite_assignment_storage;
		}
	}

	// expression<bool> add_print(initializer_list<base> args)
	// {
	// 	vector<size_t> v;
	// 	for (auto it :args){
	// 		v.push_back(it.self);
	// 	}
	// 	auto exp = expression(*this, function<T(ppp&,size_t self)>([](T ra, T rb) -> T {
	// 		return ra;
	// 	}));

	// 	this->cout_bind_to_expression.emplace(exp, v);
	// 	for(auto element : v){
	// 		if (const string* pval = get_if<0>(&element))
	//         	std::cout << "variant value: " << *pval << '\n';
	//     	else
	//         	std::cout << "failed to get value!" << '\n';
	// 	}
	// 	vars.insert(exp.self);
	// 	exp.bind_expr2(other, other, exp);
	// 	return exp;
	// }
};

int main(int argc, char* argv[])
{
	auto start = chrono::high_resolution_clock::now();
	ppp p;
	p.print_thread_info = true;
	auto xp = p.add_variable(1.);
	auto yp = p.add_variable(5.);
	auto gamma = p.add_variable(0.);
	p.for_loop(1., 5., 1.,
		function<void(ppp&, double)>(
			[&](ppp& pp, double k) {
				auto q = pp.add_variable(k);
				gamma = gamma + q;
				pp.calculate();
			}));
	// p.cout() << 1;
	p.calculate();
	string s = "a";
	string b = "b";
	// cout << p.exps.size() << endl;
	// cout << static_cast<ppp::expression<double> *>(p.exps.at(4))->result()<< endl;
	auto end = chrono::high_resolution_clock::now();
	auto diff = end - start;
	cout << chrono::duration<double, chrono::milliseconds::period>(diff).count() << " ms - total execution time" << endl;
	return 0;
}