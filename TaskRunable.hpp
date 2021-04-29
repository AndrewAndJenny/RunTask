#pragma once
#ifndef TASKRUNABLE_HPP
#define TASKRUNABLE_HPP
#include"QtCore/QRunnable"
#include "QtCore/QThreadPool"
#include "QtCore/QProcess"
#include "QtCore/QStringList"
#include "string"
#include "iostream"
#include "chrono"
#include "ctime"
#include"boost/algorithm/string.hpp"
#include "stdlib.h"
#include "qdebug.h"
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

	//system(_cmdLine.c_str());

	program = QString::fromStdString(commandSplit[0]);
	for(int i=1;i< commandSplit.size();i++)
		arguments << QString::fromStdString(commandSplit[i]);
    auto start_time = std::chrono::high_resolution_clock::now();

	QProcess process;
	process.setProgram(program);
	process.setArguments(arguments);
	process.start();
	//process.startDetached(program,arguments);
	if(process.waitForStarted()){
	    std::cout<< _id << "program start"<<std::endl;
	}
	process.waitForReadyRead(-1);
	process.waitForBytesWritten(-1);

	if(process.waitForFinished(-1)){
        std::cout<< _id << "program finish"<<std::endl;
	};
    qDebug()<<"ID:"<<_id<<" "<<QString::fromLocal8Bit(process.readAllStandardError());


    auto end_time = std::chrono::high_resolution_clock::now();
    auto cost_time = std::chrono::duration_cast<std::chrono::seconds>(end_time-start_time);
	std::cout << "Finished " << _id << " subtask. the cost of time is " << cost_time.count()<<"s"<<std::endl;
}
#endif
