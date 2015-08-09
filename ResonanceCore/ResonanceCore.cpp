// ResonanceCore.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ResonanceCore.h"
#include "Coordinator.h"
#include <exception>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//
extern Coordinator* pGlobalTheCoordinator;
extern ParameterPack* theParameterPack;

// external dll interface for interface to managed code
// notes: use BOOL here instead of bool
// need to have the .h file for use of the calling side

// Create basic object
extern "C" _declspec(dllexport) void __stdcall resonanceCoreCreate()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pGlobalTheCoordinator = new Coordinator();
}

// This is the initialize (begin state) for all sessions
extern "C" _declspec(dllexport) int __stdcall resonanceCoreInitialize( TCHAR * localStorageURL )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return pGlobalTheCoordinator->initialize( localStorageURL );
}

extern "C" _declspec(dllexport) int __stdcall resonanceCoreBeginSession( TCHAR * audioFileURL, TCHAR * imagePath )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	try
	{
		pGlobalTheCoordinator->beginSession( audioFileURL, imagePath );
	} catch (exception& e)
	{
		if ( pTheLogger != nullptr )
			pTheLogger->fatal( _T("caught exception"), CString( e.what() ) );
		return FALSE;
	}
	return TRUE;
}

// Notes: on managed side, use, re MSDN
// extern "C" __declspec(dllexport) char *resonanceCoreGetLegend(void);
// IntPtr ptr = resonanceCoreGetLegend(); 
// string str = Marshal.PtrToStringAuto(ptr);
// OR
//extern "C" __declspec(dllexport) char*  __stdcall StringReturnAPI01()
//DllImport("<path to DLL>", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
//[return: MarshalAs(UnmanagedType.LPStr)]
//public static extern string StringReturnAPI01();

extern "C" _declspec(dllexport) char * __stdcall resonanceCoreGetLegend()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//CStringA legend;		// not wide chars
	//theParameterPack->getJSONActiveLegend( &legend );
	CStringA status = theParameterPack->statusAsJSON();
	pTheLogger->info( _T("legend ="), CString(status) );
	return status.GetBuffer();
}

//
// Notes: on managed side, use, re MSDN
// extern static public void resonanceCoreInputOption( [MarshalAs(UnmanagedType.LPStr)]String^, ...
//
extern "C" _declspec(dllexport) int __stdcall resonanceCoreInputOption( TCHAR * key, TCHAR * value )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theParameterPack->inputOption( CStringA(key), CStringA(value) );
}

// Notes: on managed side, use, re MSDN, by generalization!
//..., safe_cast<double>(value) )
extern "C" _declspec(dllexport) int __stdcall resonanceCoreInputParam( TCHAR * key, double value )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theParameterPack->inputParam( CStringA(key), value );
}

extern "C" _declspec(dllexport) int __stdcall resonanceCoreEndSession()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return pGlobalTheCoordinator->endSession();
}

// Shut down and any final reporting goes here
extern "C" _declspec(dllexport) void __stdcall resonanceCoreEnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pGlobalTheCoordinator->end();
}

extern "C" _declspec(dllexport) void __stdcall resonanceCoreDestroy()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (pGlobalTheCoordinator != nullptr )
	{
		delete pGlobalTheCoordinator;
		pGlobalTheCoordinator = nullptr;
	}
}
