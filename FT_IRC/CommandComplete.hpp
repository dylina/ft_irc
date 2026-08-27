#pragma once

#include <string>
#include <vector>
#include <iterator>

class CommandComplete
{
	public:
		CommandComplete();
		std::string command;
		std::vector<std::string> params;

		bool _isDone;
	private:
		std::string prefix;
		std::string trailing;
};
