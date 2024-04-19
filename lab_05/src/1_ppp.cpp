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

		void copy_base_from(const base* other)
		{
			// context = other->context;
			self = other->self;
			next = other->next;
			prev = other->prev;
			_is_calculated = other->_is_calculated;
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

		virtual void copy_result_from(base* other) = 0;
		virtual base* deep_copy() = 0;
		virtual deque<size_t> execute() = 0;
		virtual ~base()
		{
		}
	};

	size_t aa = 0;
	unordered_map<size_t, base*> exps;
	unordered_set<size_t> vars;
	unordered_map<size_t, size_t> overwrite_assignment_storage;
	// unordered_map<size_t, vector<variant<string, size_t>>> cout_bind_to_expression;
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
				return true;
			});
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
			self = ++context.aa;
			context.exps.emplace(self, this);
		}
		expression(ppp& context, ftype<T> f, T result)
			: base { context }
			, _result { result }
			, f { f }
		{
			_is_calculated = true;
			self = ++context.aa;

			context.exps.emplace(self, this);
		}
		// rule of 3
		expression(expression&other) = default;
		// expression(expression& other):
		// base{other.context},
		// f{ftype<T>([](ppp& p, size_t id) {
		// 		const auto& prev = p.exps.at(id)->prev;
		// 		assert(!prev.empty());
		// 		const auto iter_last_calculated = prev.begin();
		// 		const auto* last_computed = p.exps.at(*iter_last_calculated);
		// 		const auto res = static_cast<const expression<T>*>(last_computed)->result();
		// 		return res;
		// 	})}
		// {
		// 	_is_calculated = false;
		// 	self = ++context.aa;
		// 	context.exps.emplace(self, this);

		// 	// assignment expression
		// 	auto* exp = new expression(
		// 		other.context,
		// 		ftype<T>([](ppp& p, size_t id) {
		// 			const auto& prev = p.exps.at(id)->prev;
		// 			assert(!prev.empty());
		// 			const auto iter_last_calculated = prev.begin();
		// 			const auto* last_computed = p.exps.at(*iter_last_calculated);
		// 			const auto res = static_cast<const expression<T>*>(last_computed)->result();
		// 			return res;
		// 		}));

		// 	base::bind_expr(*exp, *this);
		// 	context.vars.insert(this->self);
		// }
		expression operator=(const expression& other)
		{
			// if (other.self == self)
			// return *this;

			// if (find(other.prev.begin(), other.prev.end(), self) != other.prev.end()) {
			// 	context.overwrite_assignment_storage.emplace(self, other.self);
			// }
			// auto* exp = new expression(context,
			// 	ftype<T>([=](ppp& p, size_t id) {
			// 		return static_cast<expression*>(p.exps.at(p.exps.at(id)->prev[0]))->result();
			// 	}));

			// bind_expr(other, *exp);
			// this->copy_base_from(exp);
			// <- error on closure capture

			// this->prev.push_back(other.self);
			// return *this;

			context.vars.erase(self);
			context.vars.insert(other.self);
			// copy
			this->copy_base_from(&other);
			_result = other._result;
			f = other.f;
			return *this;
		}
		~expression()
		{
		}
		// ! rule of 3
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

		deque<size_t> execute() override
		{
			auto start = chrono::high_resolution_clock::now();
			_result = f(context, self);
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
			// unwrap var_wrapper

			auto lhs_expr = static_cast<expression*>(lhs.context.exps.at(lhs.prev.at(0)));
			auto rhs_expr = static_cast<expression*>(lhs.context.exps.at(lhs.prev.at(0)));

			auto* exp = new expression(
				lhs_expr.context,
				ftype<T>([=](ppp& p, size_t id) {
					const auto& prev = p.exps.at(id)->prev;
					const auto ra = static_cast<expression*>(p.exps.at(prev[0]))->result();
					const auto rb = static_cast<expression*>(p.exps.at(prev[1]))->result();
					const auto res = f(ra, rb);
					return res;
				}));
			auto* var_wrapper = new expression(
				lhs_expr.context,
				ftype<T>([](ppp& p, size_t id) {
					const auto& prev = p.exps.at(id)->prev;
					assert(!prev.empty());
					// get last calculated expression
					const auto res = static_cast<expression<T>*>(p.exps.at(prev[0]))->result();
					return res;
				}));
			bind_expr2(lhs_expr, rhs_expr, *exp);
			bind_expr(*exp, *var_wrapper);
			lhs_expr.context.vars.insert(var_wrapper->self);
			return *var_wrapper;
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

		static expression process_tmp_variable(expression& e)
		{
			e.context.vars.erase(e.self);
			auto prev = e.prev[0];
			e.context.exps.erase(e.self);
			delete &e;
			return prev;
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
		auto* exp = new expression(
			*this,
			ftype<T>([](ppp& p, size_t id) {
				cout << "never call" << endl;
				return T {};
			}),
			t);
		auto* var_wrapper = new expression(
			*this,
			ftype<T>([](ppp& p, size_t id) {
				const auto& prev = p.exps.at(id)->prev;
				assert(!prev.empty());
				const auto iter_last_calculated = prev.begin();
				const auto* last_computed = p.exps.at(*iter_last_calculated);
				const auto res = static_cast<const expression<T>*>(last_computed)->result();
				return res;
			}));

		base::bind_expr(*exp, *var_wrapper);
		vars.insert(var_wrapper->self);
		return *var_wrapper;
	}

	template <typename T>
	void for_loop(T a, T b, T c, function<void(ppp&, T)> g)
	{
		// limit the repeat count:
		size_t limit = size_t(1e9);
		size_t counter = 0;
		while (a < b) {
			g(*this, c);
			a += c;
			++counter;
			if (counter > limit)
				break;
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

// support ppp usage via defines
#define start_ppp ppp p;
#define end_ppp p.calculate();
#define var(v) p.add_variable((v));
#define start_pfor(varname, a, b, c) p.for_loop(a, b, c,function<void(ppp&, decltype((a)))>([&](ppp& p, decltype(a) varname) {
#define end_pfor \
	}));
#define pcout p.add_printer()
#define pfout(filename) p.add_fprinter((filename))
// example

// a <- 0;
// q <- 0.;
// [w<~1:3:5]{w <-a+q;"f";print('a',"beta",w); file_print<"file.txt">('b',"test",a);}
// print(w);
// #endppp;

// returns success
bool ppp_interpreter(string program_code)
{

	return true;
}

int main(int argc, char* argv[])
{
	auto start = chrono::high_resolution_clock::now();
	{
		ppp p;
		p.print_thread_info = true;
		auto x = p.add_variable(1.); // 1. -> var -> x
		auto y = p.add_variable(5.); // 5. -> var -> y
		auto a = x; // copy constructor as x -> a
		a = y; // copy assignment as y -> a
		// p.for_loop(1., 5., 1.,
		// 	function<void(ppp&, double)>(
		// 		[&](ppp& pp, double k) {
		// 			auto q = pp.add_variable(k);
		// 			gamma = gamma + q;
		// 			p.add_printer() << gamma << pp.add_variable(" test_string\n");

		// 			pp.calculate();
		// 		}));
		p.calculate();
		string s = "a";
		string b = "b";
		// p.add_printer() << xp;
		// p.add_fprinter("text.txt") << xp;
		// p.add_fprinter("text2.txt") << xp;
		// p.add_fprinter("text3.txt") << xp;
		// p.add_fprinter("text4.txt") << xp;
		p.calculate();
	}

	// // test defines
	// {
	// 	start_ppp
	// 	auto a = var(1.)
	// 	auto b = var(1.)
	// 	start_pfor(k,0,10,1)
	// 		auto q = a;
	// 		a=b;
	// 		b=q+a;
	// 	end_pfor
	// 	pcout << a;
	// 	pfout("fibonacci.out") << a;
	// 	end_ppp
	// }
	auto end = chrono::high_resolution_clock::now();
	auto diff = end - start;
	cout << chrono::duration<double, chrono::milliseconds::period>(diff).count() << " ms - total execution time" << endl;
	return 0;
}