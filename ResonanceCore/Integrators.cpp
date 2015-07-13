// Integrators.cpp : Definitions for the signal processing objects
//

#include "stdafx.h"
#include "ResonanceCoreProcess.h"
#include "Integrators.h"
#include <math.h>

//----------------------------------------------------------------------------------------------------------------


Gamma1Integrator::Gamma1Integrator( size_t Size, double tau, double samplingInterval, 
									RS_INTEGRATION_BY_FREQUENCY byFrequency, DVECTOR *pFrequencies )
{
	// Set up IIR difference equation constants. These apply to all elements of real and of complex vectors
	// z-transfrom results from audiospectrum.nb
	// 1 , a1={1,-E^(-(dt/tau))}
	// b0={N}

	// Series sum give exact numerical normalization so the integration is 1:1
	// = 1/(-1 + E^(dt/tau))
	processMemoryTime = 0;

	double dt = samplingInterval;
		
	// Assign coefficients
	a1.assign( Size, 0.0 );
	b0.assign( Size, 0.0 );
	for ( size_t i = 0; i < Size; i++ )
	{
		// Tau is constant, or larger for lower frequencies
		double thisTau = tau;
		if ( byFrequency == INTEGRATION_BY_CYCLES )
		{
			double f = (*pFrequencies)[i];
			double period = f > 0.0 ? 1.0 / f : 0.0;
			thisTau = period * tau; //max( 2.0 * period, tau );
		}

		double ntau = dt / thisTau;
		double seriesSum = 1.0 / (exp( ntau ) - 1.0 );
		double norm = 1.0 / seriesSum;
		a1[i] = -exp( -samplingInterval / tau );
		b0 [i] = norm;
		processMemoryTime = max( processMemoryTime, thisTau );
	}

	// allocate and zero out the memories
	cy1.assign( Size, CD(0.0,0.0) );
	out.assign( Size, CD(0.0,0.0) );
	powerOut.assign( Size, 0.0 );

	dy1.assign( Size, 0.0 );
}

// Perfom two integrations for two outputs: the complex integration and the matched integration of the norm.
// Use a difference equation IIR filter. 
BOOL Gamma1Integrator::Process(CVECTOR *pin)
{
	for ( size_t i = 0; i < pin->size(); i++ )
	{
		out[i] = b0[i] * (*pin)[i] - a1[i] * cy1[i];
	}

	// Push down the vectors that are the memories.
	cy1 = out;
	return TRUE;
}

BOOL Gamma1Integrator::ProcessNorm(DVECTOR *pin)
{
	for ( size_t i = 0; i < pin->size(); i++ )
	{

		powerOut[i] = b0[i] * (*pin)[i] - a1[i] * dy1[i];
	}

	// Push down the vectors that are the memories.
	dy1 = powerOut;
	return TRUE;
}

void Gamma1Integrator::test( DVECTOR *pFrequencies )
{
	DVECTOR intest;
	DVECTOR results;

	intest.assign( 1, 0.0 );
	results.assign( 100 ,0.0 );
	//Initialize( 1, 1.0, 1.0 / 8000.0, INTEGRATION_FLAT, pFrequencies );
	intest[0] = 1.0;
	for ( int i = 0; i < 100; i++ )
	{
		ProcessNorm( &intest );
		results[i] = powerOut[0];
		intest[0] = 0.0;
	}
}

Gamma1Integrator::~Gamma1Integrator()
{
	// not sure if this is needed, but it can't hurt.
	cy1.clear();
	dy1.clear();
	out.clear();
	powerOut.clear();
}
//----------------------------------------------------------------------------------------------------------------

Gamma2Integrator::Gamma2Integrator( size_t Size, double tau, double samplingInterval, 
									RS_INTEGRATION_BY_FREQUENCY byFrequency, DVECTOR *pFrequencies )
{


	// Set up IIR difference equation constants. These apply to all elements of real and of complex vectors
	// z-transfrom results from audiospectrum.nb
	// 1 , a1, a2 = {1, -2 E^(-dt/tau), E^(-(2 dt)/tau)}
	// b0, b1 = {0,  dt E^(-dt/tau)}}

	// Series sum give exact numerical normalization so the integration is 1:1
	//dt exp(dt/tau) / (-1 + exp(dt/tau))^2
	processMemoryTime = 0.0;

	double dt = samplingInterval;

	// Assign coefficients
	a1.assign( Size, 0.0 );
	a2.assign( Size, 0.0 );
	b1.assign( Size, 0.0 );
	for ( size_t i = 0; i < Size; i++ )
	{
		// Tau is constant, or larger for lower frequencies
		double thisTau = tau;
		if ( byFrequency == INTEGRATION_BY_CYCLES )
		{
			double f = (*pFrequencies)[i];
			double period = f > 0.0 ? 1.0 / f : 0.0;
			thisTau = period * tau; //max( period, tau );
		}
		double ntau = dt / thisTau;

		double seriesSum = dt * exp( ntau ) / pow(( exp( ntau ) - 1.0 ), 2);
		double norm = 1.0 / seriesSum;
		double k = exp( -samplingInterval / thisTau );
		a1[i] = -2.0 * k;
		a2[i] = k * k;
		b1[i] = norm * samplingInterval * k;

		double peakT = 2.0 * thisTau;		// This is the approximate time to peak.
		processMemoryTime = max( processMemoryTime, peakT );
	}

	// allocate and zero out the memories
	cy1.assign( Size, CD(0.0,0.0) );
	cy2.assign( Size, CD(0.0,0.0) );
	cx1.assign( Size, CD(0.0,0.0) );
	out.assign( Size, CD(0.0,0.0) );
	powerOut.assign( Size, 0.0 );

	dy1.assign( Size, 0.0 );
	dy2.assign( Size, 0.0 );
	dx1.assign( Size, 0.0 );
}

// Perfom two integrations for two outputs: the complex integration and the matched integration of the norm.
// Use a difference equation IIR filter. 
BOOL Gamma2Integrator::Process(CVECTOR *pin)
{
	for ( size_t i = 0; i < pin->size(); i++ )
	{
		out[i] = b1[i] * cx1[i] - ( a1[i] * cy1[i] + a2[i] * cy2[i] );
	}

	// Push down the vectors that are the memories.
	cx1 = *pin;
	cy2 = cy1;
	cy1 = out;
	return TRUE;
}

BOOL Gamma2Integrator::ProcessNorm(DVECTOR *pin)
{
	for ( size_t i = 0; i < pin->size(); i++ )
	{
		powerOut[i] = b1[i] * dx1[i] - ( a1[i] * dy1[i] + a2[i] * dy2[i] );
	}

	// Push down the vectors that are the memories.
	dx1 = *pin;
	dy2 = dy1;
	dy1 = powerOut;
	return TRUE;
}

Gamma2Integrator::~Gamma2Integrator()
{
	// not sure if this is needed, but it can't hurt.
	cx1.clear();
	cy1.clear();
	cy2.clear();
	dx1.clear();
	dy1.clear();
	dy2.clear();
	out.clear();
	powerOut.clear();
}
//----------------------------------------------------------------------------------------------------------------
/*
// Computing more of the PHi matrix requires combining the DCP and the Gamma2Integrator steps more elaborately
PhiForTwoPoles::PhiForTwoPoles()
{
}

BOOL PhiForTwoPoles::Initialize( size_t Size, double tau, double samplingInterval )
{
	dcp10.assign( Size, CD(0.0,0.0) );
	dcp20.assign( Size, CD(0.0,0.0) );
	dcp21.assign( Size, CD(0.0,0.0) );
//	dcp30.assign( Size, CD(0.0,0.0) );
//	dcp31.assign( Size, CD(0.0,0.0) );
//	dcp32.assign( Size, CD(0.0,0.0) );

	lag1.assign( Size, CD(0.0,0.0) );
	lag2.assign( Size, CD(0.0,0.0) );
//	lag3.assign( Size, CD(0.0,0.0) );

	pPhi00 = new Gamma2Integrator();

	pPhi10 = new Gamma2Integrator();
	pPhi11 = new Gamma2Integrator();

	pPhi20 = new Gamma2Integrator();
	pPhi21 = new Gamma2Integrator();
	pPhi22 = new Gamma2Integrator();

//	pPhi30 = new Gamma2Integrator();
//	pPhi31 = new Gamma2Integrator();
//	pPhi32 = new Gamma2Integrator();
//	pPhi33 = new Gamma2Integrator();

	pPhi00->Initialize( Size, tau, samplingInterval);

	pPhi10->Initialize( Size, tau, samplingInterval);
	pPhi11->Initialize( Size, tau, samplingInterval );

	pPhi20->Initialize( Size, tau, samplingInterval );
	pPhi21->Initialize( Size, tau, samplingInterval );
	pPhi22->Initialize( Size, tau, samplingInterval );

//	pPhi30->Initialize( Size, samplingInterval);
//	pPhi31->Initialize( Size, samplingInterval );
//	pPhi32->Initialize( Size, samplingInterval );
//	pPhi33->Initialize( Size, samplingInterval );

	return TRUE;
}

BOOL PhiForTwoPoles::Process( CVECTOR *pin )
{
	for ( size_t i = 0; i < pin->size(); i++ )
	{
		dcp10[i] = lag1[i] * conj((*pin)[i]);
		dcp20[i] = lag2[i] * conj((*pin)[i]);
		dcp21[i] = lag2[i] * conj(lag1[i]);
//		dcp30[i] = lag3[i] * conj((*pin)[i]);
//		dcp31[i] = lag3[i] * conj(lag1[i]);
//		dcp32[i] = lag3[i] * conj(lag2[i]);
	}
	pPhi00->ProcessNorm( pin );

	pPhi10->Process( &dcp10 );
	pPhi11->ProcessNorm( &lag1 );

	pPhi20->Process( &dcp20 );
	pPhi21->Process( &dcp21 );
	pPhi22->ProcessNorm( &lag2 );

//	pPhi30->Process( &dcp30 );
//	pPhi31->Process( &dcp31 );
//	pPhi32->Process( &dcp32 );
//	pPhi33->ProcessNorm( &lag3 );

//	lag3 = lag2;
	lag2 = lag1;
	lag1 = *pin;
	return TRUE;
}

PhiForTwoPoles::~PhiForTwoPoles()
{
	lag1.clear();
	lag2.clear();
//	lag3.clear();
	delete pPhi00;

	delete pPhi10;
	delete pPhi11;

	delete pPhi20;
	delete pPhi21;
	delete pPhi22;

//	delete pPhi30;
//	delete pPhi31;
//	delete pPhi32;
//	delete pPhi33;

	dcp10.clear();
	dcp20.clear();
	dcp21.clear();
//	dcp30.clear();
//	dcp31.clear();
//	dcp32.clear();
}
*/

