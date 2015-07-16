// ResonanceCore.h : external header file for the ResonanceCore DLL
//

#pragma once

//#ifndef __AFXWIN_H__
//	#error "include 'stdafx.h' before including this file for PCH"
//#endif

//#include "resource.h"		// main symbols

//
// external dll interface for interface to managed code
// need to have the .h file for use of the calling side

// Create basic object
extern "C" _declspec(dllexport) void __stdcall resonanceCoreCreate();

// This is the initialize (begin state) for all sessions
extern "C" _declspec(dllexport) void __stdcall resonanceCoreInitialize( TCHAR * localStorageURL );

extern "C" _declspec(dllexport) int __stdcall resonanceCoreBeginSession( TCHAR * audioFileURL );

//
extern "C" _declspec(dllexport) char * __stdcall resonanceCoreGetLegend();

//
// Notes: on managed side, use, re MSDN
// extern static public void resonanceCoreInputOption( [MarshalAs(UnmanagedType.LPStr)]String, ...
//
extern "C" _declspec(dllexport) void __stdcall resonanceCoreInputOption( TCHAR * key, TCHAR * value );

// Notes: on managed side, use, re MSDN, by generalization!
//..., safe_cast<double>(value) )
extern "C" _declspec(dllexport) void __stdcall resonanceCoreInputParam( TCHAR * key, double value );

//extern "C" _declspec(dllexport) void __stdcall resonanceCoreCompute(/*?? needed??*/);
extern "C" _declspec(dllexport) void __stdcall resonanceCoreEndSession();

// Shut down and any final reporting goes here
extern "C" _declspec(dllexport) void __stdcall resonanceCoreEnd();

extern "C" _declspec(dllexport) void __stdcall resonanceCoreDestroy();

