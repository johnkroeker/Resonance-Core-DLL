// ParameterPack.cpp : implementation of the ParameterPack class
//
// to do: get notification of user-change (or poll )
//	then add to map, checking for equality of parameter first-- this then drives intelligently re-computation.

#include "stdafx.h"
#include "ResonanceCore.h"
#include "Coordinator.h"
#include "ParameterPack.h"
#include <cstring>


ParameterPack::ParameterPack()
{
	// Define the parameters and set their default values
	parameters.clear();
	parameters[ CStringA("bandwidth display scale") ]		= 	0.5;
	parameters[ CStringA("display bitmap height") ]		= 	1000;
	parameters[ CStringA("display bitmap width") ]		= 	1000;
	parameters[ CStringA("endtime") ]					= 	0.01;
	parameters[ CStringA("starttime") ]					= 	0.0;
	parameters[ CStringA("fft order") ]					= 	10;
	parameters[ CStringA("fft fractional stepsize") ]		= 	0.1;
	parameters[ CStringA("high frequency limit") ]		= 	10000;
	parameters[ CStringA("histogram low cut fraction") ]	= 	0.005;
	parameters[ CStringA("histogram high cut fraction") ]	= 	0.01;
	parameters[ CStringA("integration time constant") ]	= 	2;
	parameters[ CStringA("locality scale") ]				= 	1;
	parameters[ CStringA("low frequency limit") ]			= 	80;
	parameters[ CStringA("max bandwidth") ]				= 	400;
	parameters[ CStringA("min frequency spacing") ]		= 	8;
	parameters[ CStringA("noise floor") ]					= 	10;
	parameters[ CStringA("requested number of filters") ] = 	100;
	parameters[ CStringA("snapshot time") ]				= 	0;

	// Fill out current preset choices
	presetChoice.clear();
	presetChoice[ CStringA("birdsong") ]			= RS_BIRDSONG;
	presetChoice[ CStringA("blood pressure") ]	= RS_BLOODPRESSURE;
	presetChoice[ CStringA("dolphin cochlea") ]	= RS_DOLPHIN;
	presetChoice[ CStringA("music") ]				= RS_MUSIC;
	presetChoice[ CStringA("speech") ]			= RS_SPEECH;

	strategyChoice.clear();
	strategyChoice[ CStringA("standard") ]		= RS_STANDARD;
	strategyChoice[ CStringA("quick look") ]		= RS_QUICKLOOK;
	strategyChoice[ CStringA("presentation") ]	= RS_PRESENTATION;

	// Define the frequency scale choices
	scaleChoice.clear();
	scaleChoice[ CStringA("linear") ]		= SCALE_LINEAR;
	scaleChoice[ CStringA("logarithmic") ] = SCALE_LOG10;
	scaleChoice[ CStringA("mel scale") ] = SCALE_MEL;
	scaleChoice[ CStringA("no grid") ]	= SCALE_NONE;

	// Define the poles choices
	polesChoice.clear();
	polesChoice[ CStringA("power only solution") ]		= RS_POLES_0;
	polesChoice[ CStringA("use one pole solution") ]		= RS_POLES_1;
	polesChoice[ CStringA("use two pole solution") ]		= RS_POLES_2;
	polesChoice[ CStringA("use three pole solution") ]	= RS_POLES_3;

	// Define the integration scale choices
	integrationChoice.clear();
	integrationChoice[ CStringA("first order coherence") ] = COHERENCE_ORDER_ONE;
	integrationChoice[ CStringA("second order coherence") ] = COHERENCE_ORDER_TWO;
	integrationChoice[ CStringA("no integration") ]		= INTEGRATION_NONE;

	// Define the integration frequency dependence choices
	integrationFrequency.clear();
	integrationFrequency[ CStringA("constant over frequency") ]			= INTEGRATION_FLAT;
	integrationFrequency[ CStringA("integrate fixed number of cycles") ] = INTEGRATION_BY_CYCLES;
	
	// Define the display choices
	displayChoice.clear();
	displayChoice[ CStringA("resonance spectrum") ]		= RS_RESONANCE_SPECTRUM;
	displayChoice[ CStringA("reassignment spectrum") ]	= RS_REASSIGNMENT_SPECTRUM;
	displayChoice[ CStringA("power spectrum") ]			= RS_POWER_SPECTRUM;
	
	// Define the algorithm choices
	algorithmChoice.clear();
	algorithmChoice[ CStringA("gamma four pole") ]	= GAMMA_4POLE;
	algorithmChoice[ CStringA("gamma one pole") ]		= GAMMA_1POLE;
	algorithmChoice[ CStringA("gaussian window") ]	= GAUSSIAN_WINDOW;
	
	// Define the graph choices
	graphChoice.clear();
	graphChoice[ CStringA("no graph") ]				= RS_GRAPH_NONE;
	graphChoice[ CStringA("center frequencies") ]		= GRAPH_CENTERS;
	graphChoice[ CStringA("bandwidths") ]				= GRAPH_BANDWIDTH;
	graphChoice[ CStringA("integration envelope") ]	= GRAPH_INTEGRATION_ENVELOPE;
	graphChoice[ CStringA("filter envelope") ]		= GRAPH_FILTER_ENVELOPE;
	graphChoice[ CStringA("current level distribution") ] = GRAPH_LEVEL_DISTRIBUTION;
	
	// Define the snap choices
	snapChoice.clear();
	snapChoice[ CStringA("no snap display") ] = SNAP_NONE;
	snapChoice[ CStringA("energy") ]			= SNAP_ENERGY;
	snapChoice[ CStringA("entropy") ]		=	 SNAP_ENTROPY;
	snapChoice[ CStringA("indication") ]		= SNAP_INDICATION;
	snapChoice[ CStringA("poles") ]			= SNAP_POLES;
	snapChoice[ CStringA("frequency deviation") ] = SNAP_DEVIATION;
	snapChoice[ CStringA("convergence") ]		= SNAP_CONVERGENCE;
	snapChoice[ CStringA("time shift") ]		= SNAP_TIMESHIFT;

	//Define the FFT choices
	fftChoice.clear();
	fftChoice[ CStringA("do fft") ]			= RS_FFT_NONE;
	fftChoice[ CStringA("do fft") ]			= RS_DO_FFT;
	fftChoice[ CStringA("do fft limited") ]	= RS_FFT_LIMITED;

	//Define the options and set their default enum values
	options.clear();
	options[ CStringA("resonance algorithm choice") ]	= GAMMA_4POLE;
	options[ CStringA("display choice") ]				= RS_REASSIGNMENT_SPECTRUM;
	options[ CStringA("frequency scale") ]			= SCALE_LINEAR;
	options[ CStringA("fft power spectrum") ]			= RS_FFT_NONE;
	options[ CStringA("graph choice") ]				= RS_GRAPH_NONE;
	options[ CStringA("integration algorithm choice") ] = COHERENCE_ORDER_TWO;
	options[ CStringA("integration frequency dependence") ] = INTEGRATION_BY_CYCLES;
	options[ CStringA("poles solution choice") ]		= RS_POLES_1;
	options[ CStringA("preset") ]					= RS_SPEECH;
	options[ CStringA("strategy") ]					= RS_QUICKLOOK;
	options[ CStringA("graph choice at snapshot") ]	= SNAP_NONE;

	// And byName
	optionsByName.clear();
	optionsByName[ CStringA("preset") ]						= CStringA("speech");
	optionsByName[ CStringA("strategy") ]					= CStringA("quick look");
	
	optionsByName[ CStringA("resonance algorithm choice") ]		= CStringA("gamma four pole");
	optionsByName[ CStringA("display choice") ]					= CStringA("reassignment spectrum");
	optionsByName[ CStringA("frequency scale") ]					= CStringA("linear");
	optionsByName[ CStringA("fft power spectrum") ]				= CStringA("no fft");
	optionsByName[ CStringA("graph choice") ]					= CStringA("no graph");
	optionsByName[ CStringA("integration algorithm choice") ]	= CStringA("second order coherence");
	optionsByName[ CStringA("integration frequency dependence") ] = CStringA("integrate fixed number of cycles");
	optionsByName[ CStringA("poles solution choice") ]			= CStringA("use one pole solution");
	optionsByName[ CStringA("graph choice at snapshot") ]		= CStringA("no snap display");

	// define the processing flags for both options and parameters
	processingFlags[ CStringA("display bitmap height") ]			= RS_BITMAP_FLAG;
	processingFlags[ CStringA("display bitmap width") ]			= RS_BITMAP_FLAG;
	processingFlags[ CStringA("graph choice") ]					= RS_GRAPH_FLAG;
	processingFlags[ CStringA("graph choice at snapshot") ]		= RS_GRAPH_FLAG;
	processingFlags[ CStringA("poles solution choice") ]			= RS_PROCESS_FLAG;
	processingFlags[ CStringA("fft power spectrum") ]			= RS_PROCESS_FLAG;
	processingFlags[ CStringA("fftdisplaylimited") ]				= RS_PROCESS_FLAG;
	processingFlags[ CStringA("frequency scale") ]				= RS_PROCESS_FLAG;
	processingFlags[ CStringA("resonance algorithm choice") ]	= RS_PROCESS_FLAG;
	processingFlags[ CStringA("integration choice") ]			=  	RS_PROCESS_FLAG;
	processingFlags[ CStringA("integration frequency dependence") ]	=  	RS_PROCESS_FLAG;
	processingFlags[ CStringA("display choice") ]				= RS_PROCESS_FLAG;
	processingFlags[ CStringA("bandwidth display scale") ]		= RS_PROCESS_FLAG;
	processingFlags[ CStringA("fftorder") ]						= RS_PROCESS_FLAG;
	processingFlags[ CStringA("fft fractional stepsize") ]		= RS_PROCESS_FLAG;
	processingFlags[ CStringA("high frequency limit") ]			= RS_PROCESS_FLAG;
	processingFlags[ CStringA("integration time constant") ]		= RS_PROCESS_FLAG;
	processingFlags[ CStringA("locality scale") ]				= RS_PROCESS_FLAG;
	processingFlags[ CStringA("low frequency limit") ]			= RS_PROCESS_FLAG;
	processingFlags[ CStringA("max bandwidth") ]					= RS_PROCESS_FLAG;
	processingFlags[ CStringA("min frequency spacing") ]			= RS_PROCESS_FLAG;
	processingFlags[ CStringA("noise floor") ]					= RS_PROCESS_FLAG;
	processingFlags[ CStringA("requested number of filters") ]	= RS_PROCESS_FLAG;
	processingFlags[ CStringA("snapshot time") ]					= RS_PROCESS_FLAG;
	processingFlags[ CStringA("histogram low cut fraction") ]	= RS_RENDER_FLAG;
	processingFlags[ CStringA("histogram high cut fraction") ]	= RS_RENDER_FLAG;
	processingFlags[ CStringA("endtime") ]						= RS_TIMECHANGE_FLAG;
	processingFlags[ CStringA("starttime") ]						= RS_TIMECHANGE_FLAG;

	filterLow = 0.0;
	filterHigh = 0.0;
	FFTLow = 0.0;
	FFTHigh = 0.0;

	ptheScale = new FrequencyScale();
	outputParams.numberOfFilters = 0;
	SetLabels();
	processHasChanged = TRUE;
	renderHasChanged = TRUE;
	bitmapHasChanged = TRUE;
}

RS_SCALE_CHOICE ParameterPack::GetScaleChoice()
{
	MAPINTIT it = options.find( CStringA("frequency scale") );
	if ( it == options.end() )
		return SCALE_LINEAR;
	else
		return (RS_SCALE_CHOICE) it->second; 
}

RS_GRAPH_CHOICE ParameterPack::GetGraphChoice()
{
	MAPINTIT it = options.find( CStringA("graph choice") );
	if ( it == options.end() )
		return RS_GRAPH_NONE;
	else
		return (RS_GRAPH_CHOICE) it->second; 
}

RS_INTEGRATION_CHOICE ParameterPack::GetIntegrationChoice()
{
	MAPINTIT it = options.find( CStringA("integration algorithm choice") );
	if ( it == options.end() )
		return INTEGRATION_NONE;
	else
		return (RS_INTEGRATION_CHOICE) it->second; 
}

RS_INTEGRATION_BY_FREQUENCY ParameterPack::GetIntegrationDependenceChoice()
{
	MAPINTIT it = options.find( CStringA("integration frequency dependence") );
	if ( it == options.end() )
		return INTEGRATION_FLAT;
	else
		return (RS_INTEGRATION_BY_FREQUENCY) it->second; 
}

RS_DISPLAY_CHOICE ParameterPack::GetDisplayChoice()
{
	MAPINTIT it = options.find( CStringA("display choice") );
	if ( it == options.end() )
		return RS_RESONANCE_SPECTRUM;
	else
		return (RS_DISPLAY_CHOICE) it->second; 
}

RS_SNAP_CHOICE ParameterPack::GetSnapChoice()
{
	MAPINTIT it = options.find( CStringA("graph choice at snapshot") );
	if ( it == options.end() )
		return SNAP_NONE;
	else
		return (RS_SNAP_CHOICE) it->second; 
}

RS_ALGORITHM ParameterPack::GetAlgorithmChoice()
{
	MAPINTIT it = options.find( CStringA("resonance algorithm choice") );
	if ( it == options.end() )
		return GAMMA_4POLE;
	else
		return (RS_ALGORITHM) it->second; 
}

RS_POLES_CHOICE ParameterPack::GetPolesChoice()
{
	MAPINTIT it = options.find( CStringA("poles solution choice") );
	if ( it == options.end() )
		return RS_POLES_1;
	else
		return (RS_POLES_CHOICE) it->second; 
}

ParameterPack::~ParameterPack()
{
	if ( ptheScale != nullptr )
		delete ptheScale;
}

// Main state control
BOOL ParameterPack::Process( double aSamplingRate, BOOL *pProcessChange, BOOL *pRenderChange, BOOL *pBitmpChange  )
{
	samplingRate = aSamplingRate;		// needed for concrete inferences, but NOT general

	// Apply constraints

	// Match FFT-Power Spectrum
	//RS_ALGORITHM ac = GetAlgorithmChoice();
	RS_DISPLAY_CHOICE dc = GetDisplayChoice();
	BOOL do_FFT = GetDoFFT();
	if ( do_FFT )
	{
		if ( dc != RS_POWER_SPECTRUM )
		{
			inputOption( CStringA("display choice"), CStringA("power spectrum") );
			inputOption( CStringA("poles solution choice"), CStringA("power only solution") );
			processHasChanged = TRUE;
		}
	} else {
		if ( dc == RS_POWER_SPECTRUM )
		{
			inputOption( CStringA("display choice"), CStringA("power spectrum") );
			inputOption( CStringA("poles solution choice"), CStringA("use one pole solution") );
			processHasChanged = TRUE;
		}
	}
	
	// Check Nyquist constraint
	double hf = GetHighFrequencyLimit();
	if ( hf > samplingRate / 2.0 )
	{
		parameters[ CStringA("high frequency limit") ] = samplingRate / 2.0;
		processHasChanged = TRUE;
	}

	if ( GetAndClearBitmapChangeFlag() )
	{
		*pBitmpChange = TRUE;
		processHasChanged = TRUE;
	} else
		*pBitmpChange = FALSE;

	if ( processHasChanged )
		SetFrequencyScale();

	// Set outputs
	*pProcessChange = processHasChanged;
	*pRenderChange = renderHasChanged;

	// And reset our state
	processHasChanged = FALSE;
	renderHasChanged = FALSE;
	bitmapHasChanged = FALSE;
	return TRUE;
}

bool ParameterPack::inputParam( CStringA key, double value )
{
	key.MakeLower();\

	CString s;
	s.Format(_T(": %7.3f"), value);
	pTheLogger->info( CString("ParameterPack"), CString("looking up parameter ") + CString(key) + s );

	MAPDOUBLEIT it = parameters.find( key );
	if ( it == parameters.end() )
	{
		// no key: can't accept
		pTheLogger->info( CString("ParameterPack"), CString("NO PARAMETER KEY") );
		return false;
	}
	else
	{
		// key exists correctly; change the value
		parameters[ key ] = value;

		// set processing flags
		RS_PROCESSING_FLAG flag = (RS_PROCESSING_FLAG) processingFlags[ key ];
		if ( flag == RS_BITMAP_FLAG ) bitmapHasChanged = TRUE;
		else if ( flag == RS_PROCESS_FLAG ) processHasChanged = TRUE;
		else if ( flag == RS_RENDER_FLAG ) renderHasChanged = TRUE;
		else if ( flag == RS_GRAPH_FLAG ) graphChanged = TRUE;
	}
	return true;
}
// Key is strict match. Value of preset or strategy will be a human-friendly phrase
bool ParameterPack::inputOption( CStringA key, CStringA value )
{
	key.MakeLower();
	value.MakeLower();
	pTheLogger->info( CString("ParameterPack"), CString("looking up option ") + CString(key) + CString(":")+CString(value) );

	MAPINTIT it = options.find( key );
	MAPSTRINGIT its = optionsByName.find( key );
	if ( its == optionsByName.end() )
	{
		// no key; can't accept
		pTheLogger->warning( CString("ParameterPack"), CString("NO OPTIONS KEY") );
		return false;
	}
	else
	{
		//its->second = value;

		// option key exists correctly; find the category and set in main map.
		if ( key == CStringA("strategy") )
		{
			setFriendlyStrategy( value );
		} else if ( key == CStringA("preset") )
		{
			setFriendlyPreset( value );

		} else if ( algorithmChoice.find( value ) != algorithmChoice.end() )
		{
			it->second = algorithmChoice[ value ];
		} else if ( displayChoice.find( value ) != displayChoice.end() )
		{
			it->second = displayChoice[ value ];
		} else if ( fftChoice.find( value ) != fftChoice.end() )
		{
			it->second = fftChoice[ value ];
		} else if ( graphChoice.find( value ) != graphChoice.end() )
		{
			it->second = graphChoice[ value ];
		} else if ( integrationChoice.find( value ) != integrationChoice.end() )
		{
			it->second = integrationChoice[ value ];
		} else if ( integrationFrequency.find( value ) != integrationFrequency.end() )
		{
			it->second = integrationFrequency[ value ];
		} else if ( polesChoice.find( value ) != polesChoice.end() )
		{
			it->second = polesChoice[ value ];
		} else if ( scaleChoice.find( value ) != scaleChoice.end() )
		{
			it->second = scaleChoice[ value ];
		} else if ( snapChoice.find( value ) != snapChoice.end() )
		{
			it->second = snapChoice[ value ];
		} else
		{
			pTheLogger->warning("ParameterPack", "no options found" );
			return false;
		}
		
		// set processing flags
		RS_PROCESSING_FLAG flag = (RS_PROCESSING_FLAG) processingFlags[ key ];
		if ( flag == RS_BITMAP_FLAG ) bitmapHasChanged = TRUE;
		else if ( flag == RS_PROCESS_FLAG ) processHasChanged = TRUE;
		else if ( flag == RS_RENDER_FLAG ) renderHasChanged = TRUE;
		else if ( flag == RS_GRAPH_FLAG ) graphChanged = TRUE;

	}
	return true;
}

// The incoming phrase must contain one of the key words/phrases we know
BOOL ParameterPack::setFriendlyPreset( CStringA value )
{
	MAPINTIT it;
	for ( it = presetChoice.begin(); it != presetChoice.end(); it++ )
	{
		CStringA possible = it->first;
		if ( value.Find( possible) > 0 )
		{
			options[ CStringA("preset") ] = it->second;
			SetPresets();
			return TRUE;
		}
	}
	pTheLogger->warning( "setFriendlyPreset", "lookup failed" );
	return FALSE;
}

// The incoming phrase must contain one of the key words/phrases we know
BOOL ParameterPack::setFriendlyStrategy( CStringA value )
{
	MAPINTIT it;
	for ( it = strategyChoice.begin(); it != strategyChoice.end(); it++ )
	{
		CStringA possible = it->first;
		if ( value.Find( possible) > 0 )
		{
			options[ CStringA("strategy") ] = it->second;
			SetStrategy();
			return TRUE;
		}
	}
	pTheLogger->warning( "setFriendlyStrategy", "lookup failed" );
	return FALSE;
}
void ParameterPack::SetPresets()
{
	
	int choice = options[ CStringA("preset") ];
	if ( choice == RS_SPEECH )
	{
		inputParam( CStringA("high frequency limit"), 6000.0 );
		inputParam( CStringA( "max bandwidth"), 600.0);

		inputParam( CStringA( "integration time constant"), 0.002);
		inputParam( CStringA( "noise floor"), -40.0);
		inputParam( CStringA( "locality scale"), 3.0);
		inputParam( CStringA( "bandwidth display scale"), 0.2);
		inputOption( CStringA( "frequency scale"), CStringA("Logarithmic"));
		processHasChanged = TRUE;
	} else if ( choice == RS_MUSIC )
	{
		inputParam( CStringA( "low frequency limit"), 20.0);
		inputParam( CStringA( "high frequency limit"), 16000.0);
		inputParam( CStringA( "requested number of filters"), 400);
		inputParam( CStringA( "min frequency spacing"), 4.0);
		inputParam( CStringA( "max bandwidth"), 200.0);

		inputParam( CStringA( "integration time constant"), 0.001);
		inputOption( CStringA( "frequency scale"),  CStringA("mel scale"));
		processHasChanged = TRUE;
		SetFrequencyScale();

	} else if ( choice == RS_BIRDSONG )
	{
		inputParam( CStringA( "low frequency limit"), 1000.0);
		inputParam( CStringA( "high frequency limit"), 18000.0);
		inputParam( CStringA( "max bandwidth"), 600.0);
		inputParam( CStringA( "integration time constant"), 0.001);
		inputOption( CStringA( "frequency scale"), CStringA("log scale"));
		processHasChanged = TRUE;
	} else if ( choice == RS_BLOODPRESSURE )
	{
		inputParam( CStringA( "high frequency limit"), 10.0);
		inputParam( CStringA( "low frequency limit"), 0.001);
		inputParam( CStringA( "requested number of filters"), 100.0);
		inputParam( CStringA( "min frequency spacing"), 0.001);
		inputParam( CStringA( "integration time constant"), 1.0);
		processHasChanged = TRUE;
	} else if ( choice == RS_DOLPHIN )
	{
		inputParam( CStringA( "high frequency limit"), 200000.0);		// this will be limited by sampling rate, so safe
		inputParam( CStringA( "max bandwidth"), 4000.0);				// about 10X human cohlear stiffness
		inputParam( CStringA( "low frequency limit"), 100.0);
		inputParam( CStringA( "requested number of filters"), 200.0);
		inputParam( CStringA( "min frequency spacing"), 100.0);
		inputOption( CStringA( "display choice"), CStringA("Reassignment spectrum"));
		inputParam( CStringA( "integration time constant"), 4.0);
		inputParam( CStringA( "bandwidth display scale"), 0.1);
		inputOption( CStringA( "frequency scale"), CStringA("Logarithmic"));
		processHasChanged = TRUE;
	}
}

void ParameterPack::SetStrategy()
{
	
	int choice = options[ CStringA( "strategy") ];
	CString s;
	s.Format(_T("choice is %d"), choice );
	pTheLogger->info( CString("SetStrategy"), s );

	if ( choice == RS_QUICKLOOK )
	{
		inputParam( CStringA( "requested number of filters"), 50);
		inputParam( CStringA( "display bitmap height"), 500);
		inputParam( CStringA( "display bitmap width"), 650);
		processHasChanged = TRUE;
	} else if ( choice == RS_STANDARD )
	{
		inputParam( CStringA( "display bitmap height"), 1000);
		inputParam( CStringA( "display bitmap width"), 1400);
		processHasChanged = TRUE;

	} else if ( choice == RS_PRESENTATION )
	{
		inputParam( CStringA( "display bitmap height"), 1000);
		inputParam( CStringA( "display bitmap width"), 2000);
		processHasChanged = TRUE;
	}

}

CStringA ParameterPack::FindKeyByInt( MAPINT *pMap, int key )
{
	MAPINTIT it;
	for ( it = pMap->begin(); it != pMap->end(); it++ )
	{
		if ( it->second == key )
			return it->first;
	}
	return CStringA( "Not found" );
}

CStringA ParameterPack::FindKeyByDouble( MAPDOUBLE *pMap, double key )
{
	MAPDOUBLEIT it;
	for ( it = pMap->begin(); it != pMap->end(); it++ )
	{
		if ( it->second == key )
			return it->first;
	}
	return CStringA( "Not found" );
}

void ParameterPack::SetFrequencyScale()
{
	double h = parameters[ CStringA("display bitmap height")];
	ptheScale->Initialize( GetScaleChoice(), 0.0, h - 1,
			GetFilterLow(), GetFilterHigh() );
}

BOOL ParameterPack::SetSpeechMode()
{
	return TRUE;
}

void ParameterPack::SetFilterLow( double val )
{
	BOOL doLimit = GetFFTDisplayLimited();
	if ( GetDoFFT() )
	{
		FFTLow = val;
		if ( doLimit ) 
			FFTLow = GetLowFrequencyLimit();
		outputParams.lowestFilterFrequency = FFTLow;
	} else
	{
		filterLow = val;
		outputParams.lowestFilterFrequency = filterLow;
	}
}

void ParameterPack::SetFilterHigh( double val )
{
	BOOL doLimit = GetFFTDisplayLimited();
	if ( GetDoFFT() )
	{
		FFTHigh = val;
		if ( doLimit ) 
			FFTHigh = GetHighFrequencyLimit();
		outputParams.highestFilterFrequency = FFTHigh;
	} else
	{
		filterHigh = val;
		outputParams.highestFilterFrequency = filterHigh;
	}
}

void ParameterPack::SetProcessMemoryTime( double t )
{
	outputParams.processMemoryTime = t;
}

double ParameterPack::GetFilterLow()
{
	if ( GetDoFFT() )
		return FFTLow;
	else
		return filterLow; 
}

double ParameterPack::GetFilterHigh()
{
	if (GetDoFFT() )
		return FFTHigh;
	else
		return filterHigh; 
}

void ParameterPack::SetNumberOfFilters( size_t n )
{
	outputParams.numberOfFilters = n;	
}

// for external access from a Windows UI
void ParameterPack::getOutputStatus( double *filterLow, double *filterHigh, double *numberOfFilters, double *processMemoryTime,
					double *samplingInterval )
{
	*filterLow = outputParams.lowestFilterFrequency;
	*filterHigh = outputParams.highestFilterFrequency;
	*numberOfFilters = outputParams.numberOfFilters;
	*processMemoryTime = outputParams.processMemoryTime;
	*samplingInterval = outputParams.samplingInterval;
}

CStringA ParameterPack::statusAsJSON()
{
	CStringA sOut;
	sOut = "{";
	CStringA s;
	s.Format( "\"lowestFilterFrequency\":%6.4f,", outputParams.lowestFilterFrequency);
	sOut = sOut + s;
	s.Format("\"highestFilterFrequency\":%6.4f,",	 outputParams.highestFilterFrequency);
	sOut = sOut + s;
	s.Format("\"numberOfFilters\":%d,", (int) outputParams.numberOfFilters);
	sOut = sOut + s;
	s.Format("\"processMemoryTime\":%6.4f,", outputParams.processMemoryTime);
	sOut = sOut + s;
	s.Format("\"samplingInterval\":%6.4f", outputParams.samplingInterval );
	sOut = sOut + s;
	sOut = sOut + "}";
	return sOut;
}

CStringA ParameterPack::paramsAsJSON()
{
	CStringA sOut;
	sOut = "{";
	MAPDOUBLEIT it;
	bool start = true;
	for ( it = parameters.begin(); it != parameters.end(); it++ )
	{
		if ( !start )
			sOut = sOut + ",";
		start = false;
		CStringA s = it->first;
		double d = it->second;
		CStringA f;
		f.Format( "\"%s\":%f", s, d );
		sOut = sOut + f;
	}
	sOut = sOut + "}";
	return sOut;
}

CStringA ParameterPack::optionsAsJSON()
{
	CStringA sOut;
	sOut = "{";
	MAPSTRINGIT it;
	bool start = true;
	for ( it = optionsByName.begin(); it != optionsByName.end(); it++ )
	{
		if ( !start )
			sOut = sOut + ",";
		start = false;
		CStringA s = it->first;
		CStringA s2 = it->second;
		CStringA f;
		f.Format( "\"%s\":\"%s\"", s, s2 );
		sOut = sOut + f;
	}
	sOut = sOut + "}";
	return sOut;
}

BOOL ParameterPack::getJSONActiveLegend( CStringA *stringLegend )
{
	CStringA sOut;
	sOut = "\"status\":";
	sOut = sOut + statusAsJSON();
	sOut = sOut + ",";
	sOut = sOut + "\"params\":";
	sOut = sOut + paramsAsJSON();
	sOut = sOut + ",";
	sOut = sOut + "\"options\":";
	sOut = sOut + optionsAsJSON();
	sOut = sOut + "";
	*stringLegend = sOut;
	return TRUE;
}

void ParameterPack::SetLabels()
{
	logLabels[ CStringA("0.001") ] = 0.001;
	logLabels[ CStringA("0.0015") ] = 0.0015;
	logLabels[ CStringA("0.002") ] = 0.002;
	logLabels[ CStringA("0.003") ] = 0.003;
	logLabels[ CStringA("0.005") ] = 0.005;
	logLabels[ CStringA("0.01") ] = 0.01;
	logLabels[ CStringA("0.015") ] = 0.015;
	logLabels[ CStringA("0.02") ] = 0.02;
	logLabels[ CStringA("0.03") ] = 0.03;
	logLabels[ CStringA("0.05") ] = 0.05;
	logLabels[ CStringA("0.1") ] = 0.1;
	logLabels[ CStringA("0.15") ] = 0.15;
	logLabels[ CStringA("0.2") ] = 0.2;
	logLabels[ CStringA("0.3") ] = 0.3;
	logLabels[ CStringA("0.5") ] = 0.5;
	logLabels[ CStringA("1.0") ] = 1.0;
	logLabels[ CStringA("1.5") ] = 1.5;
	logLabels[ CStringA("2.0") ] = 2.0;
	logLabels[ CStringA("3.0") ] = 3.0;
	logLabels[ CStringA("5.0") ] = 5.0;
	logLabels[ CStringA("10.0") ] = 10.0;
	logLabels[ CStringA("15.0") ] = 15.0;
	logLabels[ CStringA("20.0") ] = 20.0;
	logLabels[ CStringA("30.0") ] = 30.0;
	logLabels[ CStringA("50.0") ] = 50.0;
	logLabels[ CStringA("100") ] = 100.0;
	logLabels[ CStringA("150") ] = 150.0;
	logLabels[ CStringA("200") ] = 200.0;
	logLabels[ CStringA("300") ] = 300.0;
	logLabels[ CStringA("500") ] = 500.0;
	logLabels[ CStringA("1000") ] = 1000.0;
	logLabels[ CStringA("1500") ] = 1500.0;
	logLabels[ CStringA("2000") ] = 2000.0;
	logLabels[ CStringA("3000") ] = 3000.0;
	logLabels[ CStringA("5000") ] = 5000.0;
	logLabels[ CStringA("10 kHz") ] = 10000.0;
	logLabels[ CStringA("15 kHz") ] = 15000.0;
	logLabels[ CStringA("20 kHz") ] = 20000.0;
	logLabels[ CStringA("30 kHz") ] = 30000.0;
	logLabels[ CStringA("50 kHz") ] = 50000.0;
	logLabels[ CStringA("100 kHz") ] = 100000.0;
	logLabels[ CStringA("150 kHz") ] = 150000.0;
	logLabels[ CStringA("200 kHz") ] = 200000.0;
	logLabels[ CStringA("300 kHz") ] = 300000.0;
	logLabels[ CStringA("500 kHz") ] = 500000.0;
	logLabels[ CStringA("1.0 MHz") ] = 1000000.0;

	// for equal tempered scale
	melLabels[ CStringA("C0") ] = 16.35;
	melLabels[ CStringA("A0") ] = 27.50;
	melLabels[ CStringA("A1 55") ] = 55.0;
	melLabels[ CStringA("A2 110") ] = 110;
	melLabels[ CStringA("A3 220") ] = 220.0;
	melLabels[ CStringA("A4 440") ] = 440.0;
	melLabels[ CStringA("A5 880") ] = 880.0;
	melLabels[ CStringA("A6 1760") ] = 1760.0;
	melLabels[ CStringA("A7") ] = 3520.0;
	melLabels[ CStringA("A8") ] = 7040.0;
	melLabels[ CStringA("10,000") ] = 10000.0;
	melLabels[ CStringA("15,000") ] = 15000.0;
	melLabels[ CStringA("20,000") ] = 20000.0;
}

MAPDOUBLE* ParameterPack::GetLabelMap()
{
	RS_SCALE_CHOICE choice = GetScaleChoice();

	if ( choice == SCALE_LINEAR )
	{
		return nullptr;
	} else if ( choice == SCALE_LOG10 )
	{
		return &logLabels;
	} else if ( choice == SCALE_MEL )
	{
		return &melLabels;
	} else
		return nullptr;
}
FrequencyScale::FrequencyScale()
{
	Initialize( SCALE_LINEAR, 0.0, 0.0, 0.0, 0.0 );
}

void FrequencyScale::Initialize( RS_SCALE_CHOICE aChoice, double outputLow, double outputHigh, double anInputLow, double anInputHigh )
{
	choice = aChoice;
	outputMin = outputLow;
	outputMax = outputHigh;
	outputRange = outputHigh - outputLow;

	inputMin = anInputLow;
	inputMax = anInputHigh;
	inputRange = inputMax - inputMin;
	inputRange = inputRange <= 0.0 ? 1.0 : inputRange;

	if ( choice == SCALE_LINEAR )
	{
		scaledMin = inputMin;
		scaledMax = inputMax;
	} else if ( choice == SCALE_LOG10 )
	{
		scaledMin = inputMin <= 0.0 ? 0.0 : log10( inputMin );
		scaledMax = inputMax <= 0.0 ? 0.0 : log10( inputMax );
	} else if ( choice == SCALE_MEL )
	{
		scaledMin = inputMin <= 0.0 ? 0.0 : Mel( inputMin );
		scaledMax = inputMax <= 0.0 ? 0.0 : Mel( inputMax );
	} else
	{
		scaledMin = inputMin;
		scaledMax = inputMax;
	}
	scaledRange = scaledMax - scaledMin;
	scaledRange = scaledRange <= 0.0 ? 1.0 : scaledRange;
}

// for non-negative x.
double FrequencyScale::Scale( double x )
{
	double y = 0.0;
	if ( choice == SCALE_LINEAR )
	{
		y = ( x - inputMin ) / inputRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return outputMin + outputRange * y;

	} else if ( choice == SCALE_LOG10 )
	{
		y = x <= 0.0 ? 1.0 : x;
		y = (log10( y ) - scaledMin ) / scaledRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return outputMin + outputRange * y;
	} else if ( choice == SCALE_MEL )
	{
		y = x <= 0.0 ? 0.0 : x;
		y = (Mel( y ) - scaledMin ) / scaledRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return outputMin + outputRange * y;
	} else
	{
		// Default to linear
		y = ( x - inputMin ) / inputRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return outputMin + outputRange * y;
	}
}

// for non-negative x. Returns value in [0,1]
double FrequencyScale::Scale01( double x )
{
	double y = 0.0;
	if ( choice == SCALE_LINEAR )
	{
		y = ( x - inputMin ) / inputRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return y;

	} else if ( choice == SCALE_LOG10 )
	{
		y = x <= 0.0 ? 1.0 : x;
		y = (log10( y ) - scaledMin ) / scaledRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return y;
	} else if ( choice == SCALE_MEL )
	{
		y = x <= 0.0 ? 0.0 : x;
		y = (Mel( y ) - scaledMin ) / scaledRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return y;
	} else
	{
		y = ( x - inputMin ) / inputRange;
		y = min( y, 1.0 );
		y = max( y, 0.0 );
		return y;
	}

}

FrequencyScale::~FrequencyScale()
{
}

