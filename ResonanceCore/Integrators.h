// Integrators.h : Interface for the signal processing objects

// For simplicity, we can put the allocation/dealloc and the parameter passing in the constructor/destructor.
// Initialize will zero memories and so on.
// Shutdown may be useful...TBD

// If we end up with a lot of these, we can go to more general containers.
//

#pragma once
#include "ResonanceCoreProcess.h"
#include "ParameterPack.h"

//----------------------------------------------------------------------------------------------------------------
class Gamma1Integrator
{
public:
	Gamma1Integrator( size_t Size, double tau, double samplingInterval, RS_INTEGRATION_BY_FREQUENCY byFrequency, DVECTOR *frequencies );
	~Gamma1Integrator();
	BOOL Process(CVECTOR *pin);	// does complex 
	BOOL ProcessNorm(DVECTOR *pin);	// does real only
	void test( DVECTOR *pFrequencies );
	double GetProcessMemory() { return processMemoryTime; };

private:
	DVECTOR a1;
	DVECTOR b0;

	CVECTOR cy1;
	DVECTOR dy1;

	double processMemoryTime;

public:
	CVECTOR out;
	DVECTOR powerOut;
};
//----------------------------------------------------------------------------------------------------------------

class Gamma2Integrator
{
public:
	Gamma2Integrator( size_t Size, double tau, double samplingInterval, RS_INTEGRATION_BY_FREQUENCY byFrequency, DVECTOR *frequencies );
	~Gamma2Integrator();
	BOOL Process(CVECTOR *pin);	// does complex 
	BOOL ProcessNorm(DVECTOR *pin);	// does real only
	double GetProcessMemory() { return processMemoryTime; };

private:
// Coefficients
	DVECTOR a1;
	DVECTOR a2;
	DVECTOR b1;

	// Memories
	CVECTOR cy1;
	CVECTOR cy2;
	CVECTOR cx1;

	DVECTOR dy1;
	DVECTOR dy2;
	DVECTOR dx1;
	
	double processMemoryTime;

public:
	CVECTOR out;
	DVECTOR powerOut;
};
//----------------------------------------------------------------------------------------------------------------
/*
class PhiForTwoPoles
{
public:
	PhiForTwoPoles();
	~PhiForTwoPoles();
	BOOL Initialize(size_t size, double tau, double samplingInterval );
	BOOL Process( CVECTOR *pin );

private:
	CVECTOR dcp10;
	CVECTOR dcp20;
	CVECTOR dcp21;
//	CVECTOR dcp30;
//	CVECTOR dcp31;
//	CVECTOR dcp32;

	CVECTOR lag1;
	CVECTOR lag2;
//	CVECTOR lag3;

// Outputs live in the integrator objects
public:
	Gamma2Integrator		*pPhi00;

	Gamma2Integrator		*pPhi10;
	Gamma2Integrator		*pPhi11;

	Gamma2Integrator		*pPhi20;
	Gamma2Integrator		*pPhi21;
	Gamma2Integrator		*pPhi22;

//	Gamma2Integrator		*pPhi30;
//	Gamma2Integrator		*pPhi31;
//	Gamma2Integrator		*pPhi32;
//	Gamma2Integrator		*pPhi33;

public:
//	CVECTOR out;
};
*/
//----------------------------------------------------------------------------------------------------------------
