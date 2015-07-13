
// ParameterPack.h : interface of the ParameterPack class
//
#pragma once
#include "stdafx.h"
#include "ResonanceCore.h"
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

typedef map<CString,double> MAPDOUBLE;
typedef MAPDOUBLE::iterator MAPDOUBLEIT;

typedef map<CString,int> MAPINT;
typedef MAPINT::iterator MAPINTIT;

typedef map<CString,CString> MAPSTRING;
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
	bool inputOption( CString key, CString option );
	bool inputParam( CString key, double value );
	BOOL getJSONActiveLegend( CString *stringLegend );

public:

	// for Win UI interface only
	void getOutputStatus( double *filterLow, double *filterHigh, double *numberOfFilters, double *processMemoryTime,
					double *samplingInterval );

private:
	OutStatusData outputParams;
	CString inputFileURL;

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
	CString FindKeyByInt( MAPINT *pMap, int key );
	CString FindKeyByDouble( MAPDOUBLE *pMap, double key );
	CString optionsAsJSON();
	CString paramsAsJSON();
	CString statusAsJSON();
	
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

	CString GetGraphName( int id ) {return FindKeyByInt( &graphChoice, id ); };
	CString GetAlgorithmName( int id ) {return FindKeyByInt( &algorithmChoice, id ); };
	CString GetIntegrationName( int id ) {return FindKeyByInt( &integrationChoice, id ); };
	CString GetSnapName( int id ) {return FindKeyByInt( &snapChoice, id ); };
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

	double GetBandwidthDisplayScale()	{ return parameters[ CString("Bandwidth display scale") ]; };
	double GetDisplayBitmapHeight()		{ return parameters[ CString("Display bitmap height") ]; };
	double GetDisplayBitmapWidth()		{ return parameters[ CString("Display bitmap width") ]; };
	double GetDisplayDistributionLow()	{ return parameters[ CString("Histogram low cut fraction") ]; };
	double GetDisplayDistributionHigh() { return parameters[ CString("Histogram high cut fraction") ]; };
	double GetDisplayStart()				{ return parameters[ CString("Display start time") ]; };
	double GetDisplayEnd()				{ return parameters[ CString("Display end time") ]; };
	BOOL GetDoFFT()						{ return  options[ CString("FFT power spectrum") ] != RS_FFT_NONE; };
	BOOL GetFFTDisplayLimited()			{ return options[ CString("FFT power spectrum") ] == RS_FFT_LIMITED; };
	int	GetFFTOrder()					{ return (int) parameters[ CString("FFT order") ]; };
	double GetFFTStep()					{ return parameters[ CString("FFT fractional stepsize") ]; };
	double GetRequestedNumberOfFilters()		{ return parameters[ CString("Requested number of filters") ]; };
	double GetHighFrequencyLimit()		{ return parameters[ CString("High frequency limit") ]; };
	double GetIntegrationTau()			{ return parameters[ CString("Integration time constant") ]; };
	double GetLocalityScale()			{ return parameters[ CString("Locality scale") ]; };
	double GetLowFrequencyLimit()		{ return parameters[ CString("Low frequency limit") ]; };
	double GetMaxBandwidth()				{ return parameters[ CString("Max bandwidth") ]; };
	double GetMinFrequencySpacing()		{ return parameters[ CString("Min frequency spacing") ]; };
	double GetNoiseFloor()				{ return parameters[ CString("Noise floor") ]; };
	double GetSnapshotTime()				{ return parameters[ CString("Snapshot time") ]; };

};

