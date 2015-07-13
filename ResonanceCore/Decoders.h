// Decoderss.h : Interface for the signal processing objects

// For simplicity, we can put the allocation/dealloc and the parameter passing in the constructor/destructor.
// Initialize will zero memories and so on.
// Shutdown may be useful...TBD

// If we end up with a lot of these, we can go to more general containers.
//

#pragma once
#include "ResonanceCore.h"
#include "SignalProcesses.h"
//#include "ParameterPack.h"
//----------------------------------------------------------------------------------------------------------------

class DecodeToFrequency
{
public:
	DecodeToFrequency( DVECTOR *pDesignCenterFrequencies, size_t size, double noiseFloor,  double samplingInterval );
	~DecodeToFrequency();
	BOOL Initialize();
	BOOL Process(DVECTOR *pR00, CVECTOR *pR10 );
	BOOL Process(DVECTOR *pR00, CVECTOR *pR10, CVECTOR *pR20, CVECTOR *pR21 );
	BOOL Process(DVECTOR *pR00, CVECTOR *pR10, CVECTOR *pR20, CVECTOR *pR21, CVECTOR *pR30, CVECTOR *pR31, CVECTOR *pR32 );

private:
	void ProcessEntropy( DVECTOR *pR00 );
	void TwoPolesSolutionStationary2( double r11, double r22, CD r10, CD r20, CD r21 );
	void TwoPolesSolutionStationary3( double r00, double r11, double r22, CD r10, CD r20, CD r21, CD filterPole );
	double poleBandwidth( CD pole );
	double residualForOnePole(double r00, double r11, CD alpha0 );
	CD pole1;
	CD pole2;
	CD pole3;

	DVECTOR *pCenterFrequencies;
	double deltaT;
	double powerThreshold;
	double noiseFloor;

// Outputs
public:
	DVECTOR timeShift;
	DVECTOR frequency0;
	DVECTOR frequency1;
	DVECTOR frequency2;
	DVECTOR entropyIntensity;		// computed entropy, per channel
	CVECTOR poles0;
	CVECTOR poles1;
	CVECTOR poles2;
};
