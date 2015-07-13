// ResonanceCoreProcess.h : interface of the ResonanceCoreProcess class
//

#pragma once
//#include "PropertiesWnd.h"
#include "Audio.h"
#include "ParameterPack.h"
#include <complex>
#include <vector>
using namespace std ;

typedef complex<double> CD;
typedef vector<CD> CVECTOR;
typedef vector<double> DVECTOR;
typedef vector<float> FVECTOR;
typedef vector<int> IVECTOR;

#include "SignalProcesses.h"
#include "Integrators.h"
#include "Decoders.h"

class FilterBank
{
public:
	FilterBank( double samplingInterval, RS_ALGORITHM aChoice, size_t *pOutputSize, double *pMemoryTime );
	~FilterBank();

// Operations
public:
	BOOL Initialize();
	BOOL Process( double audioIn, double currentTime );
	BOOL Shutdown();

// Implementation
public:
	size_t GetOutputSize() { return outputSize; };
	double GetProcessMemoryTime() { return processMemoryTime; };
	DVECTOR *GetCenterFrequencies() { return &centers; };
	DVECTOR *GetFilterBandwidths() { return &bandwidths; };
	CVECTOR *GetFilterPoles() { return &filterPoles; };
	double GetFilterBankLow() { return lowestFrequency; };
	double GetFilterBankHigh() { return highestFrequency; };

private:
	RS_ALGORITHM algorithmChoice;
	BOOL doFFT;

	// Processes section
	FilterBankArray				*ptheFilterBankArray;
//	Gammatone1PoleArray			*ptheGammatone1PoleArray;
//	Gammatone4PoleArray			*ptheGammatone4PoleArray;
	FFTSpectrum					*ptheFFTSpectrum;

// Current parameters and documentation
	size_t outputSize;
	double processMemoryTime;		// the formal memory of the process chain. Will be used to work off start-up effects.
	double lowestFrequency;
	double highestFrequency;
	DVECTOR centers;
	DVECTOR bandwidths;
	CVECTOR filterPoles;

public:
	// Output
	CVECTOR *pOut;
};

class ConjugateProducts
{
public:
	ConjugateProducts( RS_POLES_CHOICE aChoice, size_t inputSize );
	~ConjugateProducts();

// Operations
public:
	BOOL Initialize();
	BOOL Process( CVECTOR *pIn );
	BOOL Shutdown();

// Implementation
public:
	size_t GetOutputSize() { return Size; };

private:
	RS_POLES_CHOICE polesChoice;
	size_t Size;
	CVECTOR x1;
	CVECTOR x2;
	CVECTOR x3;

	// Processes section
	DifferentialConjugateProduct		*ptheDifferentialConjugateProduct;

public:
	// Outputs
	DVECTOR R00;

	CVECTOR R10;

	CVECTOR R20;
	CVECTOR R21;

	CVECTOR R30;
	CVECTOR R31;
	CVECTOR R32;
};

class IntegrationProcess
{
public:
	IntegrationProcess( double samplingInterval, size_t aSize, RS_POLES_CHOICE aPolesChoice, RS_INTEGRATION_CHOICE aChoice, 
				RS_INTEGRATION_BY_FREQUENCY byFrequency, DVECTOR *pCenterFrequencies );
	~IntegrationProcess();

// Operations
public:
	BOOL Initialize();
	BOOL Process(  ConjugateProducts *pTheConjugateProducts );
	BOOL Shutdown();

// Implementation
public:
	size_t GetOutputSize() { return outputSize; };
	double GetProcessMemoryTime() { return processMemoryTime; };

private:
	RS_POLES_CHOICE polesChoice;
	RS_INTEGRATION_CHOICE integrationChoice;

	// Processes section
	Gamma1Integrator					*ptheGamma1Integrator00;
	Gamma1Integrator					*ptheGamma1Integrator10;
	Gamma1Integrator					*ptheGamma1Integrator20;
	Gamma1Integrator					*ptheGamma1Integrator21;
	Gamma1Integrator					*ptheGamma1Integrator30;
	Gamma1Integrator					*ptheGamma1Integrator31;
	Gamma1Integrator					*ptheGamma1Integrator32;

	Gamma2Integrator					*ptheGamma2Integrator00;
	Gamma2Integrator					*ptheGamma2Integrator10;
	Gamma2Integrator					*ptheGamma2Integrator20;
	Gamma2Integrator					*ptheGamma2Integrator21;
	Gamma2Integrator					*ptheGamma2Integrator30;
	Gamma2Integrator					*ptheGamma2Integrator31;
	Gamma2Integrator					*ptheGamma2Integrator32;

// Current parameters and documentation
	size_t outputSize;
	double processMemoryTime;		// the formal memory of the process chain. Will be used to work off start-up effects.

public:
	DVECTOR *pR00;
	CVECTOR *pR10;
	CVECTOR *pR20;
	CVECTOR *pR21;
};

class Decoder
{
public:
	Decoder( DVECTOR *pCenterFrequencies, RS_POLES_CHOICE aChoice, size_t inputSize, double noiseFloor, double samplingInterval  );
	~Decoder();

// Operations
public:
	BOOL Initialize();
	BOOL Process( IntegrationProcess *pIntegrationProcess );
	BOOL Shutdown();

// Implementation
public:
	size_t GetOutputSize() { return Size; };

private:
	RS_POLES_CHOICE polesChoice;
	size_t Size;

	// Processes section
	DecodeToFrequency	*ptheFrequencyDecoder;

public:
	// Outputs
	DVECTOR *pEntropyIntensity;
	DVECTOR *pFrequency0;
	DVECTOR *pFrequency1;
	CVECTOR *pPoles0;
	CVECTOR *pPoles1;
	DVECTOR *pTimeShift;
};


// Encapsulates the main signal processing functions and interfaces to ResonanceStudioDoc
class ResonanceStudioProcess
{
public:
	ResonanceStudioProcess( AudioSource *pTheAudioSource );
	~ResonanceStudioProcess();

// Operations
public:
	BOOL Initialize();
	BOOL Process( double audioIn/* in and out. tbd: memory edge effect a complication */, double currentTime );
	BOOL Shutdown();

// Implementation
public:
	size_t GetOutputSize() { return outputSize; };
	double GetProcessMemoryTime() { return processMemoryTime; };
	DVECTOR *GetCenterFrequencies() { return ptheFilterBank->GetCenterFrequencies(); };
	DVECTOR *GetFilterBandwidths()	{ return ptheFilterBank->GetFilterBandwidths(); };
	CVECTOR *GetFilterPoles()		{ return ptheFilterBank->GetFilterPoles(); };

	FVECTOR *GetDisplaySpectrum() { return pTheDisplaySpectrum; };
	double GetSpectrumMax() { return theSpectrumMax; };
	double GetSpectrumMin() { return theSpectrumMin; };

private:
	AudioSource *ourAudioSource;
	RS_ALGORITHM algorithmChoice;
	RS_POLES_CHOICE polesChoice;
	RS_INTEGRATION_CHOICE integrationChoice;
	RS_DISPLAY_CHOICE displayChoice;

	double snapshotTime;
	BOOL snapshotDone;

	// Processes section
	FilterBank				*ptheFilterBank;
	ConjugateProducts		*ptheConjugateProducts;
	IntegrationProcess		*ptheIntegrationProcess;
	Decoder					*ptheDecoder;
	FrequencyFlowInference	*ptheFrequencyFlowInference;
	ScaleForDisplay			*ptheScaleForDisplay;

// Current parameters and documentation
	size_t outputSize;
	double processMemoryTime;		// the formal memory of the process chain. Will be used to work off start-up effects.

	// Result from algorithm chain
	double theSpectrumMax;
	double theSpectrumMin;
	FVECTOR *pTheDisplaySpectrum;

// These will be available for snapshot
public:
	FVECTOR snapDisplay;
	CVECTOR snapPoles0;
	CVECTOR snapPoles1;
	DVECTOR snapEstimatedFrequencies0;
	DVECTOR snapEstimatedFrequencies1;
	DVECTOR snapEstimatedFrequencies2;
	DVECTOR snapWidths;
	DVECTOR snapIntensity;
	DVECTOR snapEnergy;
	DVECTOR snapConvergence;
	DVECTOR snapLocalIntensity;
	DVECTOR snapH0;
	DVECTOR snapTimeShift;

};