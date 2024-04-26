#include "2_ppp_class.hpp"
#include <boost/asio.hpp>
#include <boost/process.hpp>

struct ppp_compilator {
	static bool compile(const string& filepath)
	{
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
			out_file << program_code;
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