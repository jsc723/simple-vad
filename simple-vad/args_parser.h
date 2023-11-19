#pragma once
#include <vector>
#include <string>
#include <unordered_map>
using std::vector;
using std::string;
using std::unordered_map;

//ref: https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
class ArgsParser {
public:
	ArgsParser(int argc, char** argv) {
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(string(argv[i]));
	}
	string getCmdOption(const string& option) const {
		vector<string>::const_iterator itr;
		itr = find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
			return *itr;
		}
		static const string empty_string("");
		return empty_string;
	}
	void fillStringIfExist(const string& option, string& param) {
		string nr = getCmdOption(option);
		if (!nr.empty()) {
			param = nr;
		}
	}
	bool cmdOptionExists(const string& option) const {
		return find(this->tokens.begin(), this->tokens.end(), option)
			!= this->tokens.end();
	}
	string getLastArg() const {
		return *tokens.rbegin();
	}
	int getIntArg(const string& arg, int def = -1) {
		string nr = getCmdOption(arg);
		if (nr.size() > 0 && std::all_of(nr.begin(), nr.end(), ::isdigit))
			return atoi(nr.c_str());
		return def;
	}
	void fillIntIfExist(const string& arg, int& param) {
		string nr = getCmdOption(arg);
		if (nr.size() > 0 && std::all_of(nr.begin(), nr.end(), ::isdigit))
			param = atoi(nr.c_str());
	}
	double getDoubleArg(const string& arg, double def = -1) {
		string nr = getCmdOption(arg);
		if (nr.size() > 0 && std::all_of(nr.begin(), nr.end(),
			[](unsigned char c) {return ::isdigit(c) || c == '.'; }))
			return atof(nr.c_str());
		return def;
	}
	void fillDoubleIfExist(const string& arg, double &param) {
		string nr = getCmdOption(arg);
		if (nr.size() > 0 && std::all_of(nr.begin(), nr.end(),
			[](unsigned char c) {return ::isdigit(c) || c == '.'; }))
			param = atof(nr.c_str());
	}
private:
	vector <string> tokens;
};
