
// ParameterPack.h : interface of the ParameterPack class
//
#pragma once
#include "stdafx.h"
#include "ResonanceDefs.h"
#include <map>
#include <cstring>

enum RS_PRESET				{ RS_BIRDSONG, RS_SPEECH, RS_BLOODPRESSURE, RS_DOLPHIN, RS_MUSIC};
enum RS_STRATEGY				{ RS_STANDARD, RS_QUICKLOOK, RS_PRESENTATION };
enum RS_ALGORITHM			{ RS_ALGORITHM_NONE, GAUSSIAN_WINDOW, GAMMA_4POLE, GAMMA_1POLE };
enum RS_DISPLAY_CHOICE		{ RS_RESONANCE_SPECTRUM, RS_REASSIGNMENT_SPECTRUM, RS_POWER_SPECTRUM };
enum RS_GRAPH_CHOICE			{ RS_GRAPH_NONE, GRAPH_CENTERS, GRAPH_BANDWIDTH, GRAPH_INTEGRATION_ENVELOPE, 
								GRAPH_FILTER_ENVELOPE, GRAPH_LEVEL_DISTRIBUTION};
enum RS_INTEGRATION_BY_FREQUENCY { INTEGRATION_FLAT, INTEGRATION_BY_CYCLES };
enum RS_INTEGRATION_CHOICE	{ COHERENCE_ORDER_TWO, COHERENCE_ORDER_ONE, INTEGRATION_NONE };
enum RS_POLES_CHOICE			{ RS_POLES_0, RS_POLES_1, RS_POLES_2, RS_POLES_3 };
enum RS_SCALE_CHOICE			{ SCALE_NONE, SCALE_LINEAR, SCALE_LOG10, SCALE_MEL };
enum RS_SNAP_CHOICE			{ SNAP_NONE, SNAP_ENERGY, SNAP_ENTROPY, SNAP_INDICATION, SNAP_POLES, SNAP_DEVIATION, SNAP_CONVERGENCE, SNAP_TIMESHIFT };
enum RS_FFT_CHOICE			{ RS_FFT_NONE, RS_DO_FFT, RS_FFT_LIMITED };
enum RS_PROCESSING_FLAG		{ RS_PROCESS_FLAG, RS_BITMAP_FLAG, RS_RENDER_FLAG, RS_TIMECHANGE_FLAG, RS_GRAPH_FLAG};

typedef map<CStringA,double> MAPDOUBLE;
typedef MAPDOUBLE::iterator MAPDOUBLEIT;

typedef map<CStringA,int> MAPINT;
typedef MAPINT::iterator MAPINTIT;

typedef map<CStringA,CStringA> MAPSTRING;
typedef MAPSTRING::iterator MAPSTRINGIT;

class FrequencyScale
{
public:
	FrequencyScale();
	void Initialize( RS_SCALE_CHOICE aChoice, double outputLow, double outputHigh, double inputLow, double inputHigh );
	~FrequencyScale();
	
	double Scale( double in );
	double Scale01( double in );

private:
	RS_SCALE_CHOICE choice;
	double inputMin;
	double inputMax;
	double inputRange;

	double outputMin;
	double outputMax;
	double outputRange;

	double scaledMin;
	double scaledMax;
	double scaledRange;
	
	double Mel( double x ) { return 2595.0 * log10( 1.0 + x / 700.0 ); };
};

// should be master input/output data structure
typedef struct {

	// Current [out]
	double lowestFilterFrequency;	
	double highestFilterFrequency;
	double numberOfFilters;
	double samplingInterval;
	double processMemoryTime;
} OutStatusData;

// Meant to hold and bandy about all the the properties
class ParameterPack
{
public:
	ParameterPack();
	~ParameterPack();
	BOOL initialize();
	BOOL Process( double samplingRate, BOOL *pProcessChange, BOOL *renderChange, BOOL *bitmpChange );

	// Main external interface
	bool inputOption( CStringA key, CStringA option );
	bool inputParam( CStringA key, double value );
	BOOL getJSONActiveLegend( CStringA *stringLegend );
	CStringA optionsAsJSON();
	CStringA paramsAsJSON();
	CStringA statusAsJSON();

public:

	// for Win UI interface only
	void getOutputStatus( double *filterLow, double *filterHigh, double *numberOfFilters, double *processMemoryTime,
					double *samplingInterval );

private:
	OutStatusData outputParams;
	CStringA inputFileURL;

	// Validation and storage
	MAPDOUBLE parameters;
	MAPINT options;
	MAPINT processingFlags;
	MAPSTRING optionsByName;

// State for controlling computation
	BOOL dirty;
	BOOL processHasChanged;
	BOOL renderHasChanged;
	BOOL bitmapHasChanged;
	BOOL timeRangeHasChanged;
	BOOL graphChanged;

	double samplingRate;

	MAPINT algorithmChoice;
	MAPINT displayChoice;
	MAPINT fftChoice;
	MAPINT graphChoice;
	MAPINT integrationChoice;
	MAPINT integrationFrequency;
	MAPINT presetChoice;
	MAPINT polesChoice;
	MAPINT strategyChoice;
	MAPINT scaleChoice;
	MAPINT snapChoice;

	MAPDOUBLE logLabels;
//	LABELMAP linLabels;
	MAPDOUBLE melLabels;

	double filterLow;
	double filterHigh;
	double FFTLow;
	double FFTHigh;

	FrequencyScale *ptheScale;

private:
	void SetLabels();
	void SetPresets();
	void SetStrategy();
	BOOL setFriendlyPreset( CStringA value );
	BOOL setFriendlyStrategy( CStringA value );
	CStringA FindKeyByInt( MAPINT *pMap, int key );
	CStringA FindKeyByDouble( MAPDOUBLE *pMap, double key );
	
public:

	// Internal interface
	RS_SCALE_CHOICE GetScaleChoice();
	RS_ALGORITHM GetAlgorithmChoice();
	RS_POLES_CHOICE GetPolesChoice();
	RS_INTEGRATION_CHOICE GetIntegrationChoice();
	RS_DISPLAY_CHOICE GetDisplayChoice();
	RS_INTEGRATION_BY_FREQUENCY GetIntegrationDependenceChoice();
	RS_GRAPH_CHOICE GetGraphChoice();
	RS_SNAP_CHOICE GetSnapChoice();

	CStringA GetGraphName( int id ) {return FindKeyByInt( &graphChoice, id ); };
	CStringA GetAlgorithmName( int id ) {return FindKeyByInt( &algorithmChoice, id ); };
	CStringA GetIntegrationName( int id ) {return FindKeyByInt( &integrationChoice, id ); };
	CStringA GetSnapName( int id ) {return FindKeyByInt( &snapChoice, id ); };
	MAPDOUBLE* GetLabelMap();

	FrequencyScale *GetFrequencyScale() { return ptheScale; };
	double GetFilterLow();
	double GetFilterHigh();

	//Semantic/application methods
	BOOL SetSpeechMode();
	void SetFilterLow( double val ); 
	void SetFilterHigh( double val );
	void SetNumberOfFilters( size_t n );
	void SetProcessMemoryTime( double t );
	void SetFrequencyScale();		// force creation
	void	 SetSamplingInterval( double t ) { outputParams.samplingInterval = t; };

	//Process control
	BOOL GetAndClearTimeChangeFlag()			{ BOOL tmp = timeRangeHasChanged; timeRangeHasChanged = FALSE; return tmp; };
	BOOL GetAndClearProcessChangeFlag()		{ BOOL tmp = processHasChanged; processHasChanged = FALSE; return tmp; };
	BOOL GetAndClearRenderChangeFlag()		{ BOOL tmp = renderHasChanged; renderHasChanged = FALSE; return tmp; };
	BOOL GetAndClearBitmapChangeFlag()		{ BOOL tmp = bitmapHasChanged; bitmapHasChanged = FALSE; return tmp; };

	double GetBandwidthDisplayScale()	{ return parameters[ CStringA("bandwidth display scale") ]; };
	double GetDisplayBitmapHeight()		{ return parameters[ CStringA("display bitmap height") ]; };
	double GetDisplayBitmapWidth()		{ return parameters[ CStringA("display bitmap width") ]; };
	double GetDisplayDistributionLow()	{ return parameters[ CStringA("histogram low cut fraction") ]; };
	double GetDisplayDistributionHigh() { return parameters[ CStringA("histogram high cut fraction") ]; };
	double GetDisplayStart()				{ return parameters[ CStringA("starttime") ]; };
	double GetDisplayEnd()				{ return parameters[ CStringA("endtime") ]; };
	BOOL GetDoFFT()						{ return  options[ CStringA("fft power spectrum") ] != RS_FFT_NONE; };
	BOOL GetFFTDisplayLimited()			{ return options[ CStringA("fft power spectrum") ] == RS_FFT_LIMITED; };
	int	GetFFTOrder()					{ return (int) parameters[ CStringA("fft order") ]; };
	double GetFFTStep()					{ return parameters[ CStringA("fft fractional stepsize") ]; };
	double GetRequestedNumberOfFilters()		{ return parameters[ CStringA("requested number of filters") ]; };
	double GetHighFrequencyLimit()		{ return parameters[ CStringA("high frequency limit") ]; };
	double GetIntegrationTau()			{ return parameters[ CStringA("integration time constant") ]; };
	double GetLocalityScale()			{ return parameters[ CStringA("locality scale") ]; };
	double GetLowFrequencyLimit()		{ return parameters[ CStringA("low frequency limit") ]; };
	double GetMaxBandwidth()				{ return parameters[ CStringA("max bandwidth") ]; };
	double GetMinFrequencySpacing()		{ return parameters[ CStringA("min frequency spacing") ]; };
	double GetNoiseFloor()				{ return parameters[ CStringA("noise floor") ]; };
	double GetSnapshotTime()				{ return parameters[ CStringA("snapshot time") ]; };

};

