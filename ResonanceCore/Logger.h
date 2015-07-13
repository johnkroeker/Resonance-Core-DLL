#pragma once
#include <iostream>
#include <fstream>
#include <atlstr.h>

using namespace std;
//Debug
//Trace
//Info
//Warn
//Error
//Fatal


class Logger
{
public:
		Logger( const char * URL );
		~Logger();
public:
		bool output( CString s );
		bool debug( CString source, CString s );
		bool fatal( CString source, CString s );
		bool info( CString source, CString s );
		void setLevel( int level );

private:
	  ofstream logFile;
	  int reportLevel;

};

extern Logger* pTheLogger;