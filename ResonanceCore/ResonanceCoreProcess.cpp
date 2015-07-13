// ResonanceCoreProcess.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ResonanceCore.h"
//#include "MainFrm.h"

#include "ResonanceCoreProcess.h"
#include "Coordinator.h"
//#include "ResonanceStudioDoc.h"
//#include "OutputWnd.h"
//#include "PropertiesWnd.h"
#include "audio.h"
#include "ParameterPack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern AudioSource *theAudioSource;

// should route all stuff through here eventually
extern Coordinator* pGlobalTheCoordinator;

FilterBank::FilterBank( double samplingInterval, RS_ALGORITHM aChoice, size_t *pOutputSize, double *pMemoryTime )
{
	ptheFilterBankArray = nullptr;
//	ptheGammatone1PoleArray = nullptr;
//	ptheGammatone4PoleArray = nullptr;
	ptheFFTSpectrum = nullptr;
	algorithmChoice = aChoice;
	processMemoryTime = 0.0;
	outputSize = 0;
	lowestFrequency = 0.0;
	highestFrequency = 0.0;
	centers.clear();
	bandwidths.clear();
	filterPoles.clear();

	ParameterPack* ptheParameterPack = pGlobalTheCoordinator->getParameterPack();

	double maxBandwidth = ptheParameterPack->GetMaxBandwidth();
	double lowFrequencyLimit = ptheParameterPack->GetLowFrequencyLimit();
	double highFrequencyLimit = ptheParameterPack->GetHighFrequencyLimit();
	double requestedN = ptheParameterPack->GetRequestedNumberOfFilters();

	// Calculate even log spacing for given frequency limits.
	double logFraction = ( log( highFrequencyLimit ) - log( lowFrequencyLimit ) ) / requestedN;
	double proportionalSpacing = exp( logFraction ) - 1.0;

	double minSpacing = ptheParameterPack->GetMinFrequencySpacing();	
	doFFT = ptheParameterPack->GetDoFFT();
	if ( doFFT )
	{
		int order = (int) ptheParameterPack->GetFFTOrder();
		double step = ptheParameterPack->GetFFTStep();

		ptheFFTSpectrum = new FFTSpectrum();
		outputSize = ptheFFTSpectrum->Initialize( order, step, samplingInterval );
		bandwidths = ptheFFTSpectrum->designBandwidths;
		centers = ptheFFTSpectrum->designFrequencies;
		filterPoles = ptheFFTSpectrum->filterPoles;
		lowestFrequency = ptheFFTSpectrum->displayLowFrequency;
		highestFrequency = ptheFFTSpectrum->displayHighFrequency;
	}
	else
	{
		// This creates the filter array and so is the source of vector size
		if ( algorithmChoice == GAMMA_1POLE )
		{
			ptheFilterBankArray = new Gammatone1PoleArray();
		}
		else if ( algorithmChoice == GAUSSIAN_WINDOW )
		{
			ptheFilterBankArray = new ComplexGaussianArray();
		}
		else    //	 if ( algorithmChoice == GAMMA_4POLE )
		{
			ptheFilterBankArray = new Gammatone4PoleArray();
		}
	
		outputSize = ptheFilterBankArray->Initialize( samplingInterval, maxBandwidth, lowFrequencyLimit,
			highFrequencyLimit, proportionalSpacing, minSpacing, &processMemoryTime );

		centers = ptheFilterBankArray->designFrequencies;
		bandwidths = ptheFilterBankArray->designBandwidths;
		filterPoles = ptheFilterBankArray->filterPoles;
		lowestFrequency = ptheFilterBankArray->lowFrequency;
		highestFrequency = ptheFilterBankArray->highFrequency;
	}


	// Set outputs
	*pOutputSize = outputSize;
	*pMemoryTime = processMemoryTime;
}

FilterBank::~FilterBank()
{
	if ( ptheFilterBankArray != nullptr )		delete ptheFilterBankArray;
	if ( ptheFFTSpectrum != nullptr )				delete ptheFFTSpectrum;
}

BOOL FilterBank::Initialize()
{
	return TRUE;
}

BOOL FilterBank::Process( double audioIn, double currentTime )
{
	
	if ( doFFT )
		{
			if ( !ptheFFTSpectrum->Process( audioIn ) )
				return FALSE;
			pOut = &ptheFFTSpectrum->out;
		} 
		else
		{
			if ( !ptheFilterBankArray->Process( audioIn ) )
				return FALSE;
			pOut = &ptheFilterBankArray->out;
	}
	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------------------------------
ConjugateProducts::ConjugateProducts( RS_POLES_CHOICE aChoice, size_t inputSize )
{
	polesChoice = aChoice;
	Size = inputSize;
	Initialize();
}

BOOL ConjugateProducts::Initialize()
{
	x1.assign( Size, CD(0.0,0.0) );
	x2.assign( Size, CD(0.0,0.0) );
	x3.assign( Size, CD(0.0,0.0) );
	R00.assign( Size, 0.0 );
	R10.assign( Size, CD(0.0,0.0) );
	R20.assign( Size, CD(0.0,0.0) );
	R21.assign( Size, CD(0.0,0.0) );
	R30.assign( Size, CD(0.0,0.0) );
	R31.assign( Size, CD(0.0,0.0) );
	R32.assign( Size, CD(0.0,0.0) );
	return TRUE;
}

ConjugateProducts::~ConjugateProducts()
{
}

BOOL ConjugateProducts::Process( CVECTOR *pIn )
{
	for ( size_t i = 0; i < Size; i++ )
	{
		R00[i] = norm( (*pIn)[i] );
		if ( polesChoice == RS_POLES_0 ) continue;

		R10[i] = x1[i] * conj((*pIn)[i]);
		if ( polesChoice == RS_POLES_1 ) continue;

		R20[i] = x2[i] * conj((*pIn)[i]);
		R21[i] = x2[i] * conj(x1[i]);
		if ( polesChoice == RS_POLES_2 ) continue;

		R30[i] = x3[i] * conj((*pIn)[i]);
		R31[i] = x3[i] * conj(x1[i]);
		R32[i] = x3[i] * conj(x2[i]);

	}
	x3 = x2;
	x2 = x1;
	x1 = *pIn;
	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------------------------------
IntegrationProcess::IntegrationProcess( double samplingInterval, size_t aSize, RS_POLES_CHOICE aPolesChoice, RS_INTEGRATION_CHOICE aChoice, 
									  RS_INTEGRATION_BY_FREQUENCY byFrequency, DVECTOR *pCenterFrequencies )
{
	ptheGamma1Integrator00 = nullptr;
	ptheGamma1Integrator10 = nullptr;
	ptheGamma1Integrator20 = nullptr;
	ptheGamma1Integrator21 = nullptr;
	ptheGamma1Integrator30 = nullptr;
	ptheGamma1Integrator31 = nullptr;
	ptheGamma1Integrator32 = nullptr;

	ptheGamma2Integrator00 = nullptr;
	ptheGamma2Integrator10 = nullptr;
	ptheGamma2Integrator20 = nullptr;
	ptheGamma2Integrator21 = nullptr;
	ptheGamma2Integrator30 = nullptr;
	ptheGamma2Integrator31 = nullptr;
	ptheGamma2Integrator32 = nullptr;
	pR00 = nullptr;
	pR10 = nullptr;
	pR20 = nullptr;
	pR21 = nullptr;
	
	
	polesChoice = aPolesChoice;

	integrationChoice = aChoice;
	outputSize = aSize;
	processMemoryTime = 0.0;
	
	ParameterPack* ptheParameterPack = pGlobalTheCoordinator->getParameterPack();
	double integrationTau = ptheParameterPack->GetIntegrationTau();
	if ( integrationChoice == COHERENCE_ORDER_ONE )
	{

		ptheGamma1Integrator00 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma1Integrator10 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma1Integrator20 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma1Integrator21 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma1Integrator30 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma1Integrator31 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma1Integrator32 = new Gamma1Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		processMemoryTime = ptheGamma1Integrator10->GetProcessMemory();
	} else if ( integrationChoice == COHERENCE_ORDER_TWO )
	{
		ptheGamma2Integrator00 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma2Integrator10 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma2Integrator20 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma2Integrator21 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma2Integrator30 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma2Integrator31 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		ptheGamma2Integrator32 = new Gamma2Integrator( (size_t) outputSize, integrationTau, samplingInterval, byFrequency, pCenterFrequencies );
		processMemoryTime = ptheGamma2Integrator10->GetProcessMemory();
	}
	else
	{
	}
}

IntegrationProcess::~IntegrationProcess()
{
	if ( ptheGamma1Integrator00 != nullptr )		delete ptheGamma1Integrator00;
	if ( ptheGamma1Integrator10 != nullptr )		delete ptheGamma1Integrator10;
	if ( ptheGamma1Integrator20 != nullptr )		delete ptheGamma1Integrator20;
	if ( ptheGamma1Integrator21 != nullptr )		delete ptheGamma1Integrator21;
	if ( ptheGamma1Integrator30 != nullptr )		delete ptheGamma1Integrator30;
	if ( ptheGamma1Integrator31 != nullptr )		delete ptheGamma1Integrator31;
	if ( ptheGamma1Integrator32 != nullptr )		delete ptheGamma1Integrator32;

	if ( ptheGamma2Integrator00 != nullptr )		delete ptheGamma2Integrator00;
	if ( ptheGamma2Integrator10 != nullptr )		delete ptheGamma2Integrator10;
	if ( ptheGamma2Integrator20 != nullptr )		delete ptheGamma2Integrator20;
	if ( ptheGamma2Integrator21 != nullptr )		delete ptheGamma2Integrator21;
	if ( ptheGamma2Integrator30 != nullptr )		delete ptheGamma2Integrator30;
	if ( ptheGamma2Integrator31 != nullptr )		delete ptheGamma2Integrator31;
	if ( ptheGamma2Integrator32 != nullptr )		delete ptheGamma2Integrator32;
}

BOOL IntegrationProcess::Initialize()
{
	return TRUE;
}

BOOL IntegrationProcess::Process( ConjugateProducts *pTheConjugateProducts )
{
	pR00 = nullptr;
	pR10 = nullptr;
	pR20 = nullptr;
	pR21 = nullptr;

	// Different outputs are selected in each case.
	if ( integrationChoice == COHERENCE_ORDER_ONE )
	{
		if ( !ptheGamma1Integrator00->ProcessNorm( &pTheConjugateProducts->R00 ) )	return FALSE;
		pR00 = &ptheGamma1Integrator00->powerOut;
		if ( polesChoice == RS_POLES_0 ) return TRUE;

		if ( !ptheGamma1Integrator10->Process( &pTheConjugateProducts->R10 ) )		return FALSE;
		pR10 = &ptheGamma1Integrator10->out;
		if ( polesChoice == RS_POLES_1 ) return TRUE;

		if ( !ptheGamma1Integrator20->Process( &pTheConjugateProducts->R20 ) )		return FALSE;
		if ( !ptheGamma1Integrator21->Process( &pTheConjugateProducts->R21 ) )		return FALSE;
		pR20 = &ptheGamma1Integrator20->out;
		pR21 = &ptheGamma1Integrator21->out;
		if ( polesChoice == RS_POLES_2 ) return TRUE;


	} else if ( integrationChoice == COHERENCE_ORDER_TWO )
	{
		if ( !ptheGamma2Integrator00->ProcessNorm( &pTheConjugateProducts->R00 ) )	return FALSE;
		pR00 = &ptheGamma2Integrator00->powerOut;
		if ( polesChoice == RS_POLES_0 ) return TRUE;

		if ( !ptheGamma2Integrator10->Process( &pTheConjugateProducts->R10 ) )		return FALSE;
		pR10 = &ptheGamma2Integrator10->out;
		if ( polesChoice == RS_POLES_1 ) return TRUE;

		if ( !ptheGamma2Integrator20->Process( &pTheConjugateProducts->R20 ) )		return FALSE;
		if ( !ptheGamma2Integrator21->Process( &pTheConjugateProducts->R21 ) )		return FALSE;
		pR20 = &ptheGamma2Integrator20->out;
		pR21 = &ptheGamma2Integrator21->out;
		if ( polesChoice == RS_POLES_2 ) return TRUE;


	} else
	{
		pR00 = &pTheConjugateProducts->R00;
		pR10 = &pTheConjugateProducts->R10;
	}

	return TRUE;
}

Decoder::Decoder( DVECTOR *pCenterFrequencies, RS_POLES_CHOICE aChoice, size_t inputSize, double noiseFloor, double samplingInterval )
{
	polesChoice = aChoice;
	Size = inputSize;
	ptheFrequencyDecoder = new DecodeToFrequency( pCenterFrequencies, Size, noiseFloor, samplingInterval );
	pFrequency0 = nullptr;
	pFrequency1 = nullptr;
	pPoles0 = nullptr;
	pPoles1 = nullptr;
	pTimeShift = nullptr;
}

BOOL Decoder::Process(IntegrationProcess *pIntegrationProcess )
{
	pFrequency0 = nullptr;
	pFrequency1 = nullptr;
	if ( polesChoice == RS_POLES_1 || RS_POLES_0 )
	{
		ptheFrequencyDecoder->Process( pIntegrationProcess->pR00, pIntegrationProcess->pR10 );
		pFrequency0 = &ptheFrequencyDecoder->frequency0;
		pPoles0 = &ptheFrequencyDecoder->poles0;
		pPoles1;
}
	else if ( polesChoice == RS_POLES_2 )
	{
		ptheFrequencyDecoder->Process( pIntegrationProcess->pR00, pIntegrationProcess->pR10, pIntegrationProcess->pR20, pIntegrationProcess->pR21 );
		pFrequency0 = &ptheFrequencyDecoder->frequency0;
		pFrequency1 = &ptheFrequencyDecoder->frequency1;
		pPoles0 = &ptheFrequencyDecoder->poles0;
		pPoles1 = &ptheFrequencyDecoder->poles1;
	}
	//Outputs
	pEntropyIntensity = &ptheFrequencyDecoder->entropyIntensity;
	pTimeShift = &ptheFrequencyDecoder->timeShift;
	return TRUE;
}

Decoder::~Decoder() {}
BOOL Decoder::Initialize() { return TRUE; }

ResonanceStudioProcess::ResonanceStudioProcess( AudioSource *pTheAudioSource )
{

	ptheFilterBank = nullptr;
	ptheConjugateProducts = nullptr;
	ptheIntegrationProcess = nullptr;
	ptheDecoder = nullptr;
	ptheFrequencyFlowInference = nullptr;
	pTheDisplaySpectrum = nullptr;
	ptheScaleForDisplay = nullptr;

	processMemoryTime = 0.0;

	ourAudioSource = pTheAudioSource;
	ParameterPack *ptheParameterPack = pGlobalTheCoordinator->getParameterPack();

	double sampling = ourAudioSource->GetPCMSamplingRate();
	double samplingInterval = sampling > 0.0 ? 1.0/sampling : 0.0;
	ptheParameterPack->SetSamplingInterval( samplingInterval );

	snapshotTime = ptheParameterPack->GetSnapshotTime();
	snapshotDone = !( snapshotTime > 0.0 );

	double noiseFloor = ptheParameterPack->GetNoiseFloor();

	algorithmChoice = ptheParameterPack->GetAlgorithmChoice();
	polesChoice = ptheParameterPack->GetPolesChoice();
	integrationChoice = ptheParameterPack->GetIntegrationChoice();
	displayChoice = ptheParameterPack->GetDisplayChoice();
	RS_INTEGRATION_BY_FREQUENCY byFrequencyChoice = ptheParameterPack->GetIntegrationDependenceChoice();

	// These are available for snapshot
	snapEnergy.clear();
	snapEstimatedFrequencies0.clear();
	snapEstimatedFrequencies1.clear();
	snapEstimatedFrequencies2.clear();
	snapIntensity.clear();
	snapPoles0.clear(); 
	snapLocalIntensity.clear();
	snapConvergence.clear();
	snapDisplay.clear();
	snapH0.clear();
	snapTimeShift.clear();

	theSpectrumMax = 0.0;
	theSpectrumMin = 0.0;

	ptheFilterBank = new FilterBank( samplingInterval, algorithmChoice, &outputSize, &processMemoryTime );
	ptheConjugateProducts = new ConjugateProducts( polesChoice, outputSize );
	ptheIntegrationProcess = new IntegrationProcess( samplingInterval, outputSize, polesChoice, integrationChoice, byFrequencyChoice, GetCenterFrequencies() );
	processMemoryTime += ptheIntegrationProcess->GetProcessMemoryTime();

	// Return info to the ParameterPack and the rest of the system
	ptheParameterPack->SetNumberOfFilters( outputSize );
	ptheParameterPack->SetProcessMemoryTime( processMemoryTime );
	ptheParameterPack->SetFilterLow( ptheFilterBank->GetFilterBankLow() );
	ptheParameterPack->SetFilterHigh( ptheFilterBank->GetFilterBankHigh() );
	ptheParameterPack->SetFrequencyScale();		// Make sure it exists

	if ( displayChoice == RS_REASSIGNMENT_SPECTRUM || displayChoice == RS_RESONANCE_SPECTRUM )
	{

		ptheDecoder = new Decoder( ptheFilterBank->GetCenterFrequencies(),  polesChoice, outputSize, noiseFloor, samplingInterval );
		ptheDecoder->Initialize();

		ptheFrequencyFlowInference = new FrequencyFlowInference();

		size_t frequencyDisplaySize = (size_t) ptheParameterPack->GetDisplayBitmapHeight();
		double bandwidthDisplayScale = ptheParameterPack->GetBandwidthDisplayScale();
		double maxBandwidth = ptheParameterPack->GetMaxBandwidth();
		double localityScale = ptheParameterPack->GetLocalityScale();
		ptheFrequencyFlowInference->Initialize( displayChoice, ptheFilterBank->GetFilterBandwidths(), ptheFilterBank->GetCenterFrequencies(), 
			ptheParameterPack->GetFrequencyScale(),
			frequencyDisplaySize, bandwidthDisplayScale,  localityScale );

	} else if ( displayChoice == RS_POWER_SPECTRUM )
	{
		size_t frequencyDisplaySize = (size_t) ptheParameterPack->GetDisplayBitmapHeight();
		BOOL doLimit = ptheParameterPack->GetFFTDisplayLimited();
		double lowFrequencyLimit = ptheParameterPack->GetLowFrequencyLimit();
		double highFrequencyLimit = ptheParameterPack->GetHighFrequencyLimit();
		ptheScaleForDisplay = new ScaleForDisplay();
		ptheScaleForDisplay->Initialize( ptheFilterBank->GetCenterFrequencies(), ptheParameterPack->GetFrequencyScale(),
			frequencyDisplaySize, lowFrequencyLimit, highFrequencyLimit, doLimit );
	}
}

ResonanceStudioProcess::~ResonanceStudioProcess()
{
	if ( ptheFilterBank != nullptr )				delete ptheFilterBank;
	if ( ptheConjugateProducts != nullptr )		delete ptheConjugateProducts;
	if ( ptheIntegrationProcess != nullptr )		delete ptheIntegrationProcess;
	if ( ptheDecoder != nullptr )				delete ptheDecoder;
	if ( ptheFrequencyFlowInference != nullptr )	delete ptheFrequencyFlowInference;
	if ( ptheScaleForDisplay != nullptr )		delete ptheScaleForDisplay;
}

BOOL ResonanceStudioProcess::Initialize()
{
	return TRUE;
}

BOOL ResonanceStudioProcess::Shutdown()
{
	return TRUE;
}

BOOL ResonanceStudioProcess::Process( double audioIn, double currentTime )
{

	if ( !ptheFilterBank->Process( audioIn, currentTime ) )
		return FALSE;
	if ( !ptheConjugateProducts->Process( ptheFilterBank->pOut ) )
		return FALSE;
	if ( !ptheIntegrationProcess->Process( ptheConjugateProducts) )
		return FALSE;

	if ( displayChoice == RS_REASSIGNMENT_SPECTRUM || displayChoice == RS_RESONANCE_SPECTRUM )
	{
		if ( !ptheDecoder->Process( ptheIntegrationProcess ) )
			return FALSE;
		if ( !ptheFrequencyFlowInference->Process( ptheDecoder->pFrequency0, ptheDecoder->pEntropyIntensity ) )
			return FALSE;
		pTheDisplaySpectrum = &ptheFrequencyFlowInference->displaySpectrum;
		theSpectrumMax = ptheFrequencyFlowInference->spectrumMax;
		theSpectrumMin = ptheFrequencyFlowInference->spectrumMin;
	} else if ( displayChoice == RS_POWER_SPECTRUM )
	{
/*
pTheDisplaySpectrum = ptheIntegrationProcess->pfPowerOut;
		theSpectrumMax = 0.0;
		theSpectrumMin = DBL_MAX;
		for ( int i = 1; i < (int) pTheDisplaySpectrum->size(); i++ )
		{
			double d = (*pTheDisplaySpectrum)[i];
			theSpectrumMax = d != 0 ? max( theSpectrumMax, d ) : theSpectrumMax;
			theSpectrumMin = d != 0 ? min( theSpectrumMin, d ) : theSpectrumMin;
		}
*/
		if ( !ptheScaleForDisplay->Process( ptheIntegrationProcess->pR00 ) )
			return FALSE;
		pTheDisplaySpectrum = &ptheScaleForDisplay->displaySpectrum;
		theSpectrumMax = ptheScaleForDisplay->spectrumMax;
		theSpectrumMin = ptheScaleForDisplay->spectrumMin;
	}

	// Snapshot implementation. These are vector copies!
	if (!snapshotDone && currentTime >= snapshotTime )
	{
		if ( displayChoice == RS_REASSIGNMENT_SPECTRUM || displayChoice == RS_RESONANCE_SPECTRUM )
		{
			snapEstimatedFrequencies0 = *ptheDecoder->pFrequency0;
			snapTimeShift = *(ptheDecoder->pTimeShift);
			if ( ptheDecoder->pFrequency1 != nullptr )
			{
				snapEstimatedFrequencies1 = *ptheDecoder->pFrequency1;
			}
			else
			{
				snapEstimatedFrequencies1.assign( snapEstimatedFrequencies0.size(), 0.0 );
			}
			snapIntensity = *ptheDecoder->pEntropyIntensity;
			snapEnergy = *(ptheIntegrationProcess->pR00 );
			snapPoles0 = *(ptheDecoder->pPoles0);
			if ( ptheDecoder->pPoles1 != nullptr )
				snapPoles1 = *(ptheDecoder->pPoles1);
			snapWidths = ptheFrequencyFlowInference->displayWidth;
			snapConvergence = ptheFrequencyFlowInference->convergence;
			snapLocalIntensity = ptheFrequencyFlowInference->localIntensity;
			snapH0 = ptheFrequencyFlowInference->H0Entropy;
		} else
		{
			snapEstimatedFrequencies0.clear();
			snapEstimatedFrequencies1.clear();
			snapEstimatedFrequencies2.clear();
			snapEnergy = *(ptheIntegrationProcess->pR00 );
			snapIntensity = snapEnergy;
			snapPoles0.clear();
			snapPoles1.clear();
			snapWidths = *(ptheFilterBank->GetFilterBandwidths());
			snapTimeShift.clear();
		}
		snapDisplay = *pTheDisplaySpectrum;
		snapshotDone = TRUE;
	}

	return TRUE;
}
