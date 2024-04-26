#include "2_ppp_class.hpp"
#include <boost/asio.hpp>
#include <boost/process.hpp>

struct ppp_compilator {
	// static bool parse_arithmethic_type(){

	// }

	// static bool apply_operation(ppp& p, deque<string_view>& v)
	// {
	// 	auto operation = v.front();
	// 	v.pop_front();
	// 	switch (operation.at(0)) {
	// 	case '[': {
	// 		// for_loop
	// 		// [w<~1:30:5]{
	// 		auto var_name =
	// 	} break;
	// 	case '}': {
	// 		// !for_loop

	// 	} break;
	// 	default: {
	// 		if (operation.substr(0, 5) == "print") {
	// 			// cout print

	// 		} else if (operation.substr(0, 10) == "file_print") {
	// 			// file print
	// 		} else {
	// 			// arithmetic
	// 		}
	// 	}
	// 	}
	// 	return true;
	// }

	// static bool interpret_part(ppp& p, deque<string_view>& v)
	// {
	// 	for (auto operation : v)
	// 		if (!apply_operation(p, v))
	// 			return false;
	// 	return true;
	// }

	// public:

	// // returns true if parsed and executed successfully
	// static bool interpret(string program_code)
	// {
	// 	auto replace_all = [](string& str, const string& from, const string& to) {
	// 		string res;
	// 		res.reserve(str.length());
	// 		size_t start_pos = 0;
	// 		size_t pos = str.find(from, start_pos);
	// 		while (pos != string::npos) {
	// 			res += str.substr(start_pos, pos - start_pos);
	// 			res += to;
	// 			pos += from.length();
	// 			start_pos = pos;
	// 			pos = str.find(from, start_pos);
	// 		}
	// 		res += str.substr(start_pos);
	// 		str.swap(res);
	// 	};

	// 	// preprocess
	// 	// filter out the ' ', '\t' and '\n' characters
	// 	auto end_pos = end(program_code);
	// 	auto end_pos = std::remove(begin(program_code), end_pos, ' ');
	// 	auto end_pos = std::remove(begin(program_code), end_pos, '\t');
	// 	auto end_pos = std::remove(begin(program_code), end_pos, '\n');
	// 	program_code.erase(end_pos, program_code.end());

	// 	// replace '{' to '{;' and '}' to '};' for easier processing
	// 	replace_all(program_code, "{", "{;");
	// 	replace_all(program_code, "}", "};");

	// 	string_view s { program_code };
	// 	// create vector of operations with ';' as a delmiter
	// 	deque<string_view> v {};
	// 	std::string delimiter = ";";
	// 	size_t last = 0;
	// 	size_t next = s.find(delimiter, last);
	// 	while (next != string::npos) {
	// 		v.push_back(s.substr(last, next - last));
	// 		last = next + 1;
	// 	}
	// 	v.push_back(s.substr(last));
	// 	ppp p;
	// 	auto is_success = interpret_part(p, v);
	// 	return is_success;
	// }

	static bool compile(const string& filepath)
	{
		// syntax example

		// = is <~
		// a <~ 0_i32;
		// a <~ 0_i64;
		// a <~ 0_u64;
		// a <~ 0_u8;
		// q <~ 0._f64;
		// w <~ 0.9_f32;
		// |non-arithmetical|
		// b <~ true;
		// b <~ false;
		// c <~ 'a';
		// s <~ "a";
		// [w<~1:30:5]{
		// w <~ a + q;
		// print('a',"beta",w);
		// file_print<"file.txt">('b',"test"_s,a);}
		// print(w);
		// #run;
		ifstream in_file;
		in_file.open(filepath);
		std::ostringstream sstr;
		sstr << in_file.rdbuf();
		auto program_code = sstr.str();
		in_file.close();

		// transcribe to .cpp
		ofstream out_file;
		auto out_file_path = filepath;
		auto index = out_file_path.find(".ppp");
		if (index == std::string::npos)
			return false;
		out_file_path.replace(index, 4, ".cpp");
		auto compiled_file_path = filepath;
		compiled_file_path.replace(index, 4, ".exe");

		out_file.open(out_file_path);

		out_file << "#include \"2_ppp_class.hpp\"\nint main(){\n";

		// transcribe code
		{

			// auto replace_all = [](string& str, const string& from, const string& to) {
			// 	string res;
			// 	res.reserve(str.length());
			// 	size_t start_pos = 0;
			// 	size_t pos = str.find(from, start_pos);
			// 	while (pos != string::npos) {
			// 		res += str.substr(start_pos, pos - start_pos);
			// 		res += to;
			// 		pos += from.length();
			// 		start_pos = pos;
			// 		pos = str.find(from, start_pos);
			// 	}
			// 	res += str.substr(start_pos);
			// 	str.swap(res);
			// };

			// // preprocess
			// // filter out the ' ', '\t' and '\n' characters
			// auto end_pos = end(program_code);
			// end_pos = std::remove(begin(program_code), end_pos, ' ');
			// end_pos = std::remove(begin(program_code), end_pos, '\t');
			// end_pos = std::remove(begin(program_code), end_pos, '\n');
			// program_code.erase(end_pos, program_code.end());

			// no processing for simplicity
			out_file << program_code;
			// // replace '{' to '{;' and '}' to '};' for easier processing
			// replace_all(program_code, "{", "{;");
			// replace_all(program_code, "}", "};");

			// string_view s { program_code };
			// // create vector of operations with ';' as a delmiter
			// deque<string_view> v {};
			// std::string delimiter = ";";
			// size_t last = 0;
			// size_t next = s.find(delimiter, last);
			// while (next != string::npos) {
			// 	v.push_back(s.substr(last, next - last));
			// 	last = next + 1;
			// }
			// v.push_back(s.substr(last));
			// for (auto operation : v) {
			// 	out_file << operation<<';';
			// }
		}

		out_file << "return 0;\n}";
		out_file.close();

		// compile
		namespace bp = boost::process;
		namespace ba = boost::asio;

		// ba::io_service ios;
		bp::ipstream data;
		// cout << "clang++"
		// 	 << "-O3" << out_file_path << "-o" << compiled_file_path << endl;
		auto command = "clang++ -O3 " + out_file_path + " -o " + compiled_file_path;
		bp::child c(command,
			// bp::std_in.close(),
			// bp::std_err.close(),
			bp::std_out > data);
		c.wait();
		// cout << data. << endl;
		cout << "success" << endl;
		return true;
	}
};

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
	using namespace std;
	if (argc != 2) {
		cout << "Wrong number of arguments\n";
		return 1;
	}
	string filepath;
	read_arg("filepath", argv[1], filepath);
	return !ppp_compilator().compile(filepath);
}