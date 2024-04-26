#include <atomic>
#include <cassert>
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
		bool _is_calculated { false };

	public:
		ppp& context;
		size_t self;
		deque<size_t> next;
		deque<size_t> prev;
		base(ppp& context)
			: context { context }
			, self {}
			, next {}
			, prev {}
		{
		}

		bool is_calculated() const
		{
			return _is_calculated;
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

		virtual deque<size_t> execute() = 0;
		virtual ~base()
		{
		}
	};

	size_t expression_counter = 0;
	unordered_map<size_t, base*> exps;
	unordered_set<size_t> vars;
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
		for (auto v : exps) {
			if (v.second->is_calculated())
				q.emplace_back(v.first);
		}

		auto futures = vector<future<deque<size_t>>> {};
		while (!q.empty()) {
			// start read-write parallel wrap
			for (auto id : q) {

				auto* v = exps.at(id);
				futures.emplace_back(async(&base::execute, v));
			}
			q.clear();
			auto d = q;
			// fill next wrap
			for (auto& fv : futures) {
				auto vec_id = fv.get();
				for (auto id : vec_id)
					d.push_back(id);
			}

			futures.clear();
			sort(d.begin(), d.end());
			auto last = unique(d.begin(), d.end());
			// d.erase(last, d.end());

			copy_if(begin(d), last, std::back_inserter(q), [=](size_t element) {
				// check if ready to be calculated:
				auto it = exps.at(element);
				for (auto idx : it->prev)
					if (!exps.at(idx)->is_calculated())
						return false;
				// check if already calculated:
				if (it->is_calculated()) {
					return false;
				}
				return true;
			});
		}
		// free calculated expressions that are not variables
		auto k_to_delete = vector<size_t> {};
		for (auto [k, v] : exps) {
			bool is_var = vars.find(k) != vars.end();
			if (!is_var) {
				assert(v->is_calculated());
				auto& prev = v->prev;
				auto& next = v->next;
				for (auto n : next) {
					auto& np = exps.at(n)->prev;
					np.erase(find(begin(np), end(np), k));
				}
				for (auto p : prev) {
					auto& pn = exps.at(p)->next;
					pn.erase(find(begin(pn), end(pn), k));
				}
				delete v;
				k_to_delete.push_back(k);
			}
		}
		for (auto k : k_to_delete) {
			exps.erase(k);
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
			_is_calculated = false;
			self = ++context.expression_counter;
			context.exps.emplace(self, this);
		}
		expression(ppp& context, ftype<T> f, T result)
			: base { context }
			, _result { result }
			, f { f }
		{
			_is_calculated = true;
			self = ++context.expression_counter;

			context.exps.emplace(self, this);
		}
		// rule of 3
		expression(expression& other) = default;
		expression& operator=(const expression& other) = default;
		~expression()
		{
		}
		// ! rule of 3
		// expression(expression&& other) = default;
		// expression& operator=(expression&& other) = default;

		T result() const
		{
			return _result;
		}

		void result_to_osstream(ostringstream& ss) override
		{
			ss << result();
		}

		deque<size_t> execute() override
		{
			auto start = chrono::high_resolution_clock::now();
			if (!_is_calculated) {
				_result = f(context, self);
				_is_calculated = true;
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
			return next;
		}

		static auto generic_binary_operator(expression& lhs, expression& rhs, function<T(T, T)> f)
		{
			auto* exp = new expression(
				lhs.context,
				ftype<T>([=](ppp& p, size_t id) {
					const auto& prev = p.exps.at(id)->prev;
					const auto ra = static_cast<expression*>(p.exps.at(prev[0]))->result();
					const auto rb = static_cast<expression*>(p.exps.at(prev[1]))->result();
					const auto res = f(ra, rb);
					return res;
				}));
			bind_expr2(lhs, rhs, *exp);
			lhs.context.vars.insert(exp->self);
			return *exp;
		}

		friend auto operator+(expression& lhs, expression& rhs)
		{
			return generic_binary_operator(lhs, rhs, [](T a, T b) {
				const auto res = a + b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '+' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		friend auto operator-(expression& lhs, expression& rhs)
		{
			return generic_binary_operator(lhs, rhs, [](T a, T b) {
				const auto res = a - b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '-' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		friend auto operator*(expression& lhs, expression& rhs)
		{
			return generic_binary_operator(lhs, rhs, [](T a, T b) {
				const auto res = a * b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '*' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		friend auto operator/(expression& lhs, expression& rhs)
		{
			return generic_binary_operator(lhs, rhs, [](T a, T b) {
				const auto res = a / b;
#ifdef PRINT_OPERATIONS_OF_LAMBDAS
				ostringstream ss;
				ss << a << '/' << b << '=' << res << '\n';
				cout << ss.str() << flush;
#endif
				return res;
			});
		}

		// remove from variables temporary expressions
		friend auto operator+(expression&& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			lhs.context.vars.erase(rhs.self);
			return lhs + rhs;
		}
		friend auto operator+(expression&& lhs, expression& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			return lhs + rhs;
		}
		friend auto operator+(expression& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(rhs.self);
			return lhs + rhs;
		}

		friend auto operator-(expression&& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			lhs.context.vars.erase(rhs.self);
			return lhs - rhs;
		}
		friend auto operator-(expression&& lhs, expression& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			return lhs - rhs;
		}
		friend auto operator-(expression& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(rhs.self);
			return lhs - rhs;
		}

		friend auto operator*(expression&& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			lhs.context.vars.erase(rhs.self);
			return lhs * rhs;
		}
		friend auto operator*(expression&& lhs, expression& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			return lhs * rhs;
		}
		friend auto operator*(expression& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(rhs.self);
			return lhs * rhs;
		}

		// strings can only be added
		auto operator-(expression<string>&& rhs) = delete;
		auto operator-(expression<string>& rhs) = delete;
		auto operator*(expression<string>&& rhs) = delete;
		auto operator*(expression<string>& rhs) = delete;
		auto operator/(expression<string>& rhs) = delete;
		auto operator/(expression<string>&& rhs) = delete;

		friend auto operator/(expression&& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			lhs.context.vars.erase(rhs.self);
			return lhs / rhs;
		}
		friend auto operator/(expression&& lhs, expression& rhs)
		{
			lhs.context.vars.erase(lhs.self);
			return lhs / rhs;
		}
		friend auto operator/(expression& lhs, expression&& rhs)
		{
			lhs.context.vars.erase(rhs.self);
			return lhs / rhs;
		}
	};

	template <typename T>
	expression<T> add_variable(const T t)
	{
		auto* exp = new expression(
			*this,
			ftype<T>([](ppp& p, size_t id) {
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
		auto* exp = new expression(
			*this,
			ftype<T>([](ppp& p, size_t id) {
				const auto& prev = p.exps.at(id)->prev;
				assert(!prev.empty());
				const auto iter_last_calculated = prev.begin();
				const auto* last_computed = p.exps.at(*iter_last_calculated);
				const auto res = static_cast<const expression<T>*>(last_computed)->result();
				return res;
			}));
		base::bind_expr(other, *exp);
		vars.insert(exp->self);
		return *exp;
	}

	template <typename T>
	void expression_assignment(expression<T>& from, expression<T>& to)
	{
		vars.erase(to.self);
		to.self = from.self;
	}

	template <typename T>
	void expression_assignment(expression<T>&& from, expression<T>& to)
	{
		vars.erase(to.self);
		to.self = from.self;
	}

	template <typename T>
	void for_loop(T a, T b, T c, function<void(ppp&, T)> g)
	{
		// limit the repeat count:
		size_t limit = size_t(1e9);
		size_t counter = 0;
		while (a < b) {
			g(*this, a);
			a += c;
			++counter;
			if (counter > limit) {
				calculate();
				limit += size_t(1e9);
			}
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

		template <typename T>
		cout_pprinter& operator<<(T&& t)
		{
			auto exp = context.add_variable(t);
			// not a variable since rvalue
			context.vars.erase(exp.self);

			q.push_back(exp.self);
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
			context.vars.erase(exp->self);
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

		template <typename T>
		file_pprinter& operator<<(T&& t)
		{
			auto exp = context.add_variable(t);
			// not a variable since rvalue
			context.vars.erase(exp.self);

			q.push_back(exp.self);
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
			context.vars.erase(exp->self);
			context.vars.erase(path.self);
			for (auto e : q)
				base::bind_expr(*context.exps.at(e), *exp);
		}
	};
	file_pprinter add_fprinter(const string& filename)
	{
		return file_pprinter(*this, filename);
	}
};

// support ppp usage via defines
#define start_ppp ppp p;
#define ppp_show_info p.print_thread_info = true;
#define run_ppp p.calculate();
#define var(v) p.add_variable((v));
#define ppp_assign(from, to) p.expression_assignment((from), (to));
#define start_pfor(varname, a, b, c) \
	p.for_loop(a, b, c,function<void(ppp&, decltype((a)))>([&](ppp& p, decltype(a) varname) {
#define end_pfor \
	}));
#define pcout p.add_printer()
#define pfout(filename) p.add_fprinter((filename))
