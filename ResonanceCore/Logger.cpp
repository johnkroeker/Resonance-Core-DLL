// logger.cpp : implementation of the logging class
//

#include "stdafx.h"
#include "Logger.h"
#include <atlstr.h>
#include <time.h>
using namespace std;


Logger* pTheLogger = nullptr;

Logger::Logger(const char * URLName )
{
	// Open with append
	logFile.open( URLName, ios::out | ios::app );
	pTheLogger = this;
	reportLevel = 10;		// report all
}

Logger::~Logger()
{
	logFile.close();
}

bool Logger::output( CString s )
{
	time_t rawtime;
    tm timeinfo;
    char buffer [80];

    time(&rawtime);
	localtime_s(&timeinfo, &rawtime );

 //	strftime (buffer, 80, "%F %R %S ", &timeresult);
	strftime(buffer,80,"%Y-%m-%d-%H-%M-%S ", &timeinfo);	

   // Convert CString from wide to ASCII
	logFile << buffer << CT2A(s) << endl;
	return true;
}

bool Logger::debug( CString source, CString s )
{
	output ( CString("DEBUG ") + source + _T(" ") + s );
	return true;
}

bool Logger::fatal( CString source, CString s )
{
	output ( CString("FATAL ") + source + _T(" ") + s );
	return true;
}

bool Logger::info( CString source, CString s )
{
	output ( CString("INFO ") + source + _T(" ") + s );
	return true;
}

void Logger::setLevel( int level )
{
	//Debug	6
	//Trace	5
	//Info	4
	//Warn	3
	//Error	2
	//Fatal	1
	//None	0
	reportLevel = level;
}