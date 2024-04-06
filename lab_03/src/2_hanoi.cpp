#include <deque>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
using namespace std;

template <typename T>
class my_deq {
	struct node {
		T data;
		node* prev;
		node* next;
	};
	node* _begin;
	node* _end;
	size_t _size;

public:
	my_deq()
		: _size { 0 }
	{
	}

	~my_deq()
	{
		while (size())
			pop_back();
	}

	auto push_back(T&& data)
	{
		auto* new_node = new node { data, _end, nullptr };
		if (_size == 0)
			_begin = _end = new_node;
		else {
			_end->next = new_node;
			_end = new_node;
		}
		++_size;
		if (_size == 0)
			throw std::overflow_error("Pushed back too many data in my_deq");
	}

	auto push_front(T&& data)
	{
		auto* new_node = new node { data, nullptr, _begin };
		if (_size == 0)
			_begin = _end = new_node;
		else {
			_begin->prev = new_node;
			_begin = new_node;
		}
		++_size;
		if (_size == 0)
			throw std::overflow_error("Pushed front too many data in my_deq");
	}

	template <typename... Args>
	auto emplace_back(Args&&... args) { push_back(T { args... }); }

	template <typename... Args>
	auto emplace_front(Args&&... args) { push_back(T { args... }); }

	auto pop_back()
	{
		auto prev = _end->prev;
		delete _end;
		_end = prev;
		--_size;
	}

	auto pop_front()
	{
		auto next = _begin->next;
		delete _begin;
		_begin = next;
		--_size;
	}

	const auto back()
	{
		if (_size == 0)
			throw std::out_of_range("my_deq is empty, invalid operation back()");
		return _end->data;
	}

	const auto front()
	{
		if (_size == 0)
			throw std::out_of_range("my_deq is empty, invalid operation front()");
		return _begin->data;
	}

	const auto size() { return _size; }
};

template <typename T>
void calculate_hanoi(int from, int to, int x, T& g)
{
	auto third = 3 - from - to;
	if (x <= 0)
		return;
	calculate_hanoi(from, third, x - 1, g);
	g.emplace_back(from, to);
	calculate_hanoi(third, to, x - 1, g);
}

template <typename T, typename S1, typename S2>
bool read_arg(S1 name, S2 from, T& to)
{
	using namespace std;
	auto input = istringstream(from);
	cin.rdbuf(input.rdbuf());
	cin >> to;
	if (cin.fail() || !cin.eof()) {
		cout << "Wrong " << name << " argument format\n";
		return 1;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	// for debug purposes
	// argc = 2;
	// char *arg[] = {"","6"};
	// argv = arg;
	if (argc != 2) {
		cout << "Wrong number of arguments\n";
		return 1;
	}
	int n;
	if (read_arg("first", argv[1], n))
		return 1;
	// Use std::deque
	deque<pair<int, int>> g;
	calculate_hanoi(0, 2, n, g);
	while (g.size() > 0) {
		auto [a, b] = g.front();
		g.pop_front();
		cout << a << b << ' ';
	}
	cout << endl;
	// Compare with my_deq
	my_deq<pair<int, int>> w;
	calculate_hanoi(0, 2, n, w);
	while (w.size() > 0) {
		auto [a, b] = w.front();
		w.pop_front();
		cout << a << b << ' ';
	}
	return 0;
}