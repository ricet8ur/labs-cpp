#include <atomic>
#include <chrono>
#include <cmath>
#include <fstream>
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

	public:
		ppp& context;
		size_t self;
		vector<size_t> next;
		vector<size_t> prev;
		base(ppp& context)
			: context { context }
			, self {}
			, next {}
			, prev {}
		{
		}

		void copy_base_from(base* other)
		{
			// context = other->context;
			self = other->self;
			next = other->next;
			prev = other->prev;
			_calculated = other->_calculated;
		}

		bool is_calculated() const
		{
			return _calculated;
		}

		static void bind_expr(const base& from, const base& to)
		{
			from.context.exps.at(to.self)->prev.emplace_back(from.self);
			from.context.exps.at(from.self)->next.emplace_back(to.self);
		}
		static void bind_expr2(const base& lhs, const base& rhs, const base& res)
		{
			bind_expr(lhs, res);
			bind_expr(rhs, res);
		}
		virtual void result_to_osstream(ostringstream& ss) = 0;

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
	mutex file_printer_mutex;

	~ppp()
	{
		for (auto [k, v] : exps) {
			delete v;
		}
	}

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
		expression(ppp& context, ftype<T> f)
			: base { context }
			, f { f }
		{
			_calculated = false;
			self = ++context.aa;
			context.exps.emplace(self, this);
		}
		expression(ppp& context, ftype<T> f, T result)
			: base { context }
			, _result { result }
			, f { f }
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

		void result_to_osstream(ostringstream& ss) override
		{
			ss << result();
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

		expression operator=(const expression& other)
		{
			if (other.self == self)
				return *this;

			if (find(other.prev.begin(), other.prev.end(), self) != other.prev.end()) {
				context.overwrite_assignment_storage.emplace(self, other.self);
			}

			auto* exp = new expression(context,
				ftype<T>([=](ppp& p, size_t id) {
					return static_cast<expression*>(p.exps.at(p.exps.at(id)->prev[0]))->result();
				}));
			bind_expr(other, *exp);
			// this->copy_base_from(exp);
			// <- error on closure capture
			return *this;
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

		// strings can only be added
		// auto operator-(expression<basic_string<char>>&& rhs) = delete;
		// auto operator-(expression<basic_string<char>>& rhs) = delete;
		// auto operator*(expression<basic_string<char>>&& rhs) = delete;
		// auto operator*(expression<basic_string<char>>& rhs) = delete;
		// auto operator/(expression<basic_string<char>>& rhs) = delete;
		// auto operator/(expression<basic_string<char>>&& rhs) = delete;

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
				// auto & it = exps.at(k);
				exps.at(k)->copy_base_from(v);
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

	class cout_pprinter {
		vector<size_t> q;
		ppp& context;

	public:
		cout_pprinter(ppp& p)
			: context { p }
			, q {}
		{
		}
		~cout_pprinter()
		{
			finish();
		}
		template <typename T>
		cout_pprinter& operator<<(expression<T> t)
		{
			q.push_back(t.self);
			return *this;
		}
		void finish()
		{
			auto* exp = new expression<bool>(context, [](ppp& p, size_t id) -> bool {
				const auto& prev = p.exps.at(id)->prev;
				ostringstream ss;
				for (auto pid : prev) {
					p.exps.at(pid)->result_to_osstream(ss);
				}
				cout << ss.str() << flush;
				return true;
			});
			for (auto e : q)
				base::bind_expr(*context.exps.at(e), *exp);
		}
	};
	cout_pprinter add_printer()
	{
		return cout_pprinter(*this);
	}

	class file_pprinter {
		vector<size_t> q;
		ppp& context;

	public:
		string filename;
		file_pprinter(ppp& p, const string& filename)
			: context { p }
			, q {}
			, filename { filename }
		{
		}
		~file_pprinter()
		{
			finish();
		}
		template <typename T>
		file_pprinter& operator<<(expression<T> t)
		{
			q.push_back(t.self);
			return *this;
		}
		void finish()
		{
			auto path = context.add_variable(string(filename));
			auto* exp = new expression<bool>(context, [](ppp& p, size_t id) -> bool {
				const lock_guard<mutex> lock(p.file_printer_mutex);
				const auto& prev = p.exps.at(id)->prev;
				ostringstream ss;
				bool filename = 1;
				for (auto pid : prev) {
					if (filename) {
						filename = 0;
						continue;
					}
					p.exps.at(pid)->result_to_osstream(ss);
				}
				ofstream fout;
				const auto* path_exp = static_cast<expression<string>*>(p.exps.at(p.exps.at(id)->prev[0]));
				fout.open(path_exp->result(), ios_base::app);
				fout << ss.str() << flush;
				fout.close();
				return true;
			});
			base::bind_expr(path, *exp);
			for (auto e : q)
				base::bind_expr(*context.exps.at(e), *exp);
		}
	};
	file_pprinter add_fprinter(const string& filename)
	{
		return file_pprinter(*this, filename);
	}
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
				p.add_printer() << gamma << pp.add_variable(" test_string\n");

				pp.calculate();
			}));
	// p.cout() << 1;
	p.calculate();
	string s = "a";
	string b = "b";
	p.add_printer() << xp;
	p.add_fprinter("text.txt") << xp;
	p.add_fprinter("text2.txt") << xp;
	p.add_fprinter("text3.txt") << xp;
	p.add_fprinter("text4.txt") << xp;
	p.calculate();

	// cout << p.exps.size() << endl;
	// cout << static_cast<ppp::expression<double> *>(p.exps.at(4))->result()<< endl;
	auto end = chrono::high_resolution_clock::now();
	auto diff = end - start;
	cout << chrono::duration<double, chrono::milliseconds::period>(diff).count() << " ms - total execution time" << endl;
	return 0;
}