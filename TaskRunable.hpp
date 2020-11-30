#pragma once
#ifndef TASKRUNABLE_HPP
#define TASKRUNABLE_HPP
#include"QtCore/QRunnable"
#include "QtCore/QThreadPool"
#include "string"
#include"boost/algorithm/string.hpp"
#include "process.h"
class TaskRunable :public QRunnable
{
public:
	explicit TaskRunable(std::string cmd):
		cmdLine(cmd){}
	~TaskRunable() {}
	void run();

private:
	std::string cmdLine;
};

void TaskRunable::run()
{
	char* argument[50];
	std::vector<std::string> commandSplit;
	boost::split(commandSplit, cmdLine, boost::is_any_of(" "), boost::token_compress_on);

	for (int pCommandSplit = 0; pCommandSplit < commandSplit.size(); pCommandSplit++)
		argument[pCommandSplit] = const_cast<char*>(commandSplit[pCommandSplit].c_str());
	argument[commandSplit.size()] = NULL;

	execv(argument[0], argument);
}
#endif
