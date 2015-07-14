// ResonanceCore.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ResonanceCore.h"
#include "Coordinator.h"

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
extern "C" _declspec(dllexport) void __stdcall resonanceCoreCreate( char* localStorageURL )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pGlobalTheCoordinator = new Coordinator( CString(localStorageURL) );
}

extern "C" _declspec(dllexport) void __stdcall resonanceCoreBeginSession( char* audioFileURL )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pGlobalTheCoordinator->OnOpenDocument( (LPCTSTR) audioFileURL );
}

// Notes: on managed side, use, re MSDN
// extern "C" __declspec(dllexport) char *resonanceCoreGetLegend(void);
// IntPtr ptr = resonanceCoreGetLegend(); 
// string str = Marshal.PtrToStringAuto(ptr);
//
extern "C" _declspec(dllexport) char * __stdcall resonaceCoreGetLegend()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CStringA legend;		// not wide chars
	theParameterPack->getJSONActiveLegend( &legend );
	return legend.GetBuffer();
}

//
// Notes: on managed side, use, re MSDN
// extern static public void resonanceCoreInputOption( [MarshalAs(UnmanagedType::LPStr)]String^, ...
//
extern "C" _declspec(dllexport) void __stdcall resonaceCoreInputOption( char* key, char* value )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theParameterPack->inputOption( CString(key), CString(value) );
}

// Notes: on managed side, use, re MSDN, by generalization!
//..., safe_cast<double>(value) )
extern "C" _declspec(dllexport) void __stdcall resonanceCoreInputParam( char* key, double value )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theParameterPack->inputParam( CString(key), value );
}
extern "C" _declspec(dllexport) void __stdcall resonanceCoreCompute(/*?? needed??*/);
extern "C" _declspec(dllexport) void __stdcall resonanceCoreEndSession()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pGlobalTheCoordinator->OnCloseDocument();
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
