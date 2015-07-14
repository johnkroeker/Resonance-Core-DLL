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
extern "C" _declspec(dllexport) void __stdcall resonanceCoreCreate( char* localStorageURL );

extern "C" _declspec(dllexport) void __stdcall resonanceCoreBeginSession( char* audioFileURL );


// Notes: on managed side, use, re MSDN
// extern "C" __declspec(dllexport) char *resonanceCoreGetLegend(void);
// IntPtr ptr = resonanceCoreGetLegend(); 
// string str = Marshal.PtrToStringAuto(ptr);
//
extern "C" _declspec(dllexport) char * __stdcall resonaceCoreGetLegend();

//
// Notes: on managed side, use, re MSDN
// extern static public void resonanceCoreInputOption( [MarshalAs(UnmanagedType::LPStr)]String^, ...
//
extern "C" _declspec(dllexport) void __stdcall resonaceCoreInputOption( char* key, char* value );

// Notes: on managed side, use, re MSDN, by generalization!
//..., safe_cast<double>(value) )
extern "C" _declspec(dllexport) void __stdcall resonanceCoreInputParam( char* key, double value );

//extern "C" _declspec(dllexport) void __stdcall resonanceCoreCompute(/*?? needed??*/);
extern "C" _declspec(dllexport) void __stdcall resonanceCoreEndSession();

extern "C" _declspec(dllexport) void __stdcall resonanceCoreDestroy();

