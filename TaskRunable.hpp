#pragma once
#ifndef TASKRUNABLE_HPP
#define TASKRUNABLE_HPP
#include"QtCore/QRunnable"
#include "QtCore/QThreadPool"
#include "QtCore/QProcess"
#include "QtCore/QStringList"
#include "string"
#include "iostream"
#include"boost/algorithm/string.hpp"
class TaskRunable :public QRunnable
{
public:
	explicit TaskRunable(int id, std::string cmd):
		_id(id), _cmdLine(cmd){}
	~TaskRunable() {}
	void run();

private:
	int _id;
	std::string _cmdLine;
};

void TaskRunable::run()
{
	std::vector<std::string> commandSplit;
	boost::split(commandSplit, _cmdLine, boost::is_any_of(" "), boost::token_compress_on);

	QString program;
	QStringList arguments;

	program = QString::fromStdString(commandSplit[0]);
	for(int i=1;i< commandSplit.size();i++)
		arguments << QString::fromStdString(commandSplit[i]);

	QProcess process;
	process.setProgram(program);
	process.setArguments(arguments);
	process.start();
	process.waitForFinished(-1);
	std::cout << "Finished " << _id << " subtask" << std::endl;
}
#endif
