// ResonanceCore.h : main header file for the ResonanceCore DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CResonanceCoreApp
// See ResonanceCore.cpp for the implementation of this class
//

class CResonanceCoreApp : public CWinApp
{
public:
	CResonanceCoreApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#include <complex>
#include <vector>
using namespace std ;

typedef complex<double> CD;
typedef vector<CD> CVECTOR;
typedef vector<double> DVECTOR;
typedef vector<float> FVECTOR;
typedef vector<int> IVECTOR;
