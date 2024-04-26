#include "2_ppp_class.hpp"

int main(int argc, char* argv[])
{
	auto start = chrono::high_resolution_clock::now();
	{
		// test 0
		ppp p;
		p.print_thread_info = true;

		auto x = p.add_variable(1.); // 1. -> var -> x

		auto y = p.add_variable(5.); // 5. -> var -> y
		auto a = p.add_variable(x); // copy constructor as x -> a
		p.expression_assignment(y, a);
		// a = y; // copy assignment as y -> a
		// a = a + y;
		p.expression_assignment(a + y, a);
	}

	{
		ppp p;
		p.print_thread_info = true;

		auto gamma = p.add_variable(2.);
		p.for_loop(1., 5., 1.,
			function<void(ppp&, double)>(
				[&](ppp& pp, double k) {
					auto q = pp.add_variable(k);
					pp.expression_assignment(gamma + q, gamma);
					// gamma = gamma + q;
					pp.add_printer() << gamma << " test_string\n";

					// pp.calculate();
				}));
		p.add_printer() << "!for_loop\n";
		cout << 'a' << endl;
		p.calculate();
		string s = "a";
		string b = "b";
		p.add_printer() << s;
		p.add_fprinter("text.txt") << s;
		p.add_fprinter("text2.txt") << s;
		p.add_fprinter("text3.txt") << s;
		p.add_fprinter("text4.txt") << s;
		p.add_printer() << "!end\n";
		cout << 'a' << endl;
		p.calculate();
		p.add_printer() << "!end2\n";
		cout << 'a' << endl;
		p.calculate();
	}

	{
		// test fibo
		ppp p;
		p.print_thread_info = true;
		auto a = p.add_variable(1.);
		auto b = p.add_variable(1.);
		p.for_loop(0., 10., 1.,
			function<void(ppp&, double)>(
				[&](ppp& pp, double k) {
					auto q = pp.add_variable(a);
					pp.expression_assignment(b, a);
					pp.expression_assignment(q + a, b);
				}));
		p.calculate();
	}

	// test defines
	{
		start_ppp;
		auto a = var(1.);
		auto b = var(1.);
		start_pfor(k, 0, 10, 1);
		auto q = var(a);
		// a=b;
		// b=q+a;
		ppp_assign(b, a);
		ppp_assign(q + a, b);
		end_pfor;
		pcout << a;
		pfout("fibonacci.out") << a;
		end_ppp;
	}
	auto end = chrono::high_resolution_clock::now();
	auto diff = end - start;
	cout << chrono::duration<double, chrono::milliseconds::period>(diff).count() << " ms - total execution time" << endl;
	return 0;
}