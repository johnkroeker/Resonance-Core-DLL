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
	parameters[ CString("Bandwidth display scale") ]		= 	0.5;
	parameters[ CString("Display bitmap height") ]		= 	1000;
	parameters[ CString("Display bitmap width") ]		= 	1000;
	parameters[ CString("Display end time") ]			= 	0.01;
	parameters[ CString("Display start time") ]			= 	0;
	parameters[ CString("FFT order") ]					= 	10;
	parameters[ CString("FFT fractional stepsize") ]		= 	0.1;
	parameters[ CString("High frequency limit") ]		= 	10000;
	parameters[ CString("Histogram low cut fraction") ]	= 	0.005;
	parameters[ CString("Histogram high cut fraction") ]	= 	0.01;
	parameters[ CString("Integration time constant") ]	= 	2;
	parameters[ CString("Locality scale") ]				= 	1;
	parameters[ CString("Low frequency limit") ]			= 	80;
	parameters[ CString("Max bandwidth") ]				= 	400;
	parameters[ CString("Min frequency spacing") ]		= 	8;
	parameters[ CString("Noise floor") ]					= 	10;
	parameters[ CString("Requested number of filters") ] = 	100;
	parameters[ CString("Snapshot time") ]				= 	0;

	// Fill out current preset choices
	presetChoice.clear();
	presetChoice[ CString("Birdsong") ]			= RS_BIRDSONG;
	presetChoice[ CString("Blood pressure") ]	= RS_BLOODPRESSURE;
	presetChoice[ CString("Dolphin cochlea") ]	= RS_DOLPHIN;
	presetChoice[ CString("Music") ]				= RS_MUSIC;
	presetChoice[ CString("Speech") ]			= RS_SPEECH;

	strategyChoice.clear();
	strategyChoice[ CString("Standard") ]		= RS_STANDARD;
	strategyChoice[ CString("Quick look") ]		= RS_QUICKLOOK;
	strategyChoice[ CString("Presentation") ]	= RS_PRESENTATION;

	// Define the frequency scale choices
	scaleChoice.clear();
	scaleChoice[ CString("Linear") ]		= SCALE_LINEAR;
	scaleChoice[ CString("Logarithmic") ] = SCALE_LOG10;
	scaleChoice[ CString("Mel scale") ] = SCALE_MEL;
	scaleChoice[ CString("No grid") ]	= SCALE_NONE;

	// Define the poles choices
	polesChoice.clear();
	polesChoice[ CString("Power only solution") ]		= RS_POLES_0;
	polesChoice[ CString("Use one pole solution") ]		= RS_POLES_1;
	polesChoice[ CString("Use two pole solution") ]		= RS_POLES_2;
	polesChoice[ CString("Use three pole solution") ]	= RS_POLES_3;

	// Define the integration scale choices
	integrationChoice.clear();
	integrationChoice[ CString("First order coherence") ] = COHERENCE_ORDER_ONE;
	integrationChoice[ CString("Second order coherence") ] = COHERENCE_ORDER_TWO;
	integrationChoice[ CString("No integration") ]		= INTEGRATION_NONE;

	// Define the integration frequency dependence choices
	integrationFrequency.clear();
	integrationFrequency[ CString("Constant over frequency") ]			= INTEGRATION_FLAT;
	integrationFrequency[ CString("Integrate fixed number of cycles") ] = INTEGRATION_BY_CYCLES;
	
	// Define the display choices
	displayChoice.clear();
	displayChoice[ CString("Resonance spectrum") ]		= RS_RESONANCE_SPECTRUM;
	displayChoice[ CString("Reassignment spectrum") ]	= RS_REASSIGNMENT_SPECTRUM;
	displayChoice[ CString("Power spectrum") ]			= RS_POWER_SPECTRUM;
	
	// Define the algorithm choices
	algorithmChoice.clear();
	algorithmChoice[ CString("Gamma four pole") ]	= GAMMA_4POLE;
	algorithmChoice[ CString("Gamma one pole") ]		= GAMMA_1POLE;
	algorithmChoice[ CString("Gaussian window") ]	= GAUSSIAN_WINDOW;
	
	// Define the graph choices
	graphChoice.clear();
	graphChoice[ CString("No graph") ]				= RS_GRAPH_NONE;
	graphChoice[ CString("Center frequencies") ]		= GRAPH_CENTERS;
	graphChoice[ CString("Bandwidths") ]				= GRAPH_BANDWIDTH;
	graphChoice[ CString("Integration envelope") ]	= GRAPH_INTEGRATION_ENVELOPE;
	graphChoice[ CString("Filter envelope") ]		= GRAPH_FILTER_ENVELOPE;
	graphChoice[ CString("Current level distribution") ] = GRAPH_LEVEL_DISTRIBUTION;
	
	// Define the snap choices
	snapChoice.clear();
	snapChoice[ CString("No snap display") ] = SNAP_NONE;
	snapChoice[ CString("Energy") ]			= SNAP_ENERGY;
	snapChoice[ CString("Entropy") ]		=	 SNAP_ENTROPY;
	snapChoice[ CString("Indication") ]		= SNAP_INDICATION;
	snapChoice[ CString("Poles") ]			= SNAP_POLES;
	snapChoice[ CString("Frequency deviation") ] = SNAP_DEVIATION;
	snapChoice[ CString("Convergence") ]		= SNAP_CONVERGENCE;
	snapChoice[ CString("Time shift") ]		= SNAP_TIMESHIFT;

	//Define the FFT choices
	fftChoice.clear();
	fftChoice[ CString("No FFT") ]			= RS_FFT_NONE;
	fftChoice[ CString("Do FFT") ]			= RS_DO_FFT;
	fftChoice[ CString("Do FFT limited") ]	= RS_FFT_LIMITED;

	//Define the options and set their default enum values
	options.clear();
	options[ CString("Resonance algorithm choice") ]	= GAMMA_4POLE;
	options[ CString("Display choice") ]				= RS_REASSIGNMENT_SPECTRUM;
	options[ CString("Frequency scale") ]			= SCALE_LINEAR;
	options[ CString("FFT power spectrum") ]			= RS_FFT_NONE;
	options[ CString("Graph choice") ]				= RS_GRAPH_NONE;
	options[ CString("Integration algorithm choice") ] = COHERENCE_ORDER_TWO;
	options[ CString("Integration frequency dependence") ] = INTEGRATION_BY_CYCLES;
	options[ CString("Poles solution choice") ]		= RS_POLES_1;
	options[ CString("Preset choice") ]				= RS_SPEECH;
	options[ CString("Strategy choice") ]			= RS_QUICKLOOK;
	options[ CString("Graph choice at snapshot") ]	= SNAP_NONE;

	// And byName
	optionsByName.clear();
	optionsByName[ CString("Resonance algorithm choice") ]	= CString("Gamma four pole");
	optionsByName[ CString("Display choice") ]				= CString("Reassignment spectrum");
	optionsByName[ CString("Frequency scale") ]				= CString("Linear");
	optionsByName[ CString("FFT power spectrum") ]			= CString("No FFT");
	optionsByName[ CString("Graph choice") ]					= CString("No graph");
	optionsByName[ CString("Integration algorithm choice") ] = CString("Second order coherence");
	optionsByName[ CString("Integration frequency dependence") ] = CString("Integrate fixed number of cycles");
	optionsByName[ CString("Poles solution choice") ]		= CString("Use one pole solution");
	optionsByName[ CString("Preset choice") ]				= CString("Speech");
	optionsByName[ CString("Strategy choice") ]				= CString("Quick look");
	optionsByName[ CString("Graph choice at snapshot") ]		= CString("No snap display");

	// define the processing flags for both options and parameters
	processingFlags[ CString("Display bitmap height") ]		= RS_BITMAP_FLAG;
	processingFlags[ CString("Display bitmap width") ]		= RS_BITMAP_FLAG;
	processingFlags[ CString("Graph choice") ]				= RS_GRAPH_FLAG;
	processingFlags[ CString("Graph choice at snapshot") ]	= RS_GRAPH_FLAG;
	processingFlags[ CString("Poles solution choice") ]		= RS_PROCESS_FLAG;
	processingFlags[ CString("FFT power spectrum") ]			= RS_PROCESS_FLAG;
	processingFlags[ CString("FFTDisplayLimited") ]			= RS_PROCESS_FLAG;
	processingFlags[ CString("Frequency scale") ]			= RS_PROCESS_FLAG;
	processingFlags[ CString("Resonance algorithm choice") ]			= RS_PROCESS_FLAG;
	processingFlags[ CString("Integration choice") ]					=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Integration frequency dependence") ]	=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Display choice") ]				=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Bandwidth display scale") ]	=  	RS_PROCESS_FLAG;
	processingFlags[ CString("FFTOorder") ]					=  	RS_PROCESS_FLAG;
	processingFlags[ CString("FFT fractional stepsize") ]	=  	RS_PROCESS_FLAG;
	processingFlags[ CString("High frequency limit") ]		=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Integration time constant") ] =  	RS_PROCESS_FLAG;
	processingFlags[ CString("Locality scale") ]				=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Low frequency limit") ]		=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Max bandwidth") ]				=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Min frequency spacing") ]		=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Noise floor") ]				=  	RS_PROCESS_FLAG;
	processingFlags[ CString("Requested number of filters") ]	= RS_PROCESS_FLAG;
	processingFlags[ CString("Snapshot time") ]					= RS_PROCESS_FLAG;
	processingFlags[ CString("Histogram low cut fraction") ]		= RS_RENDER_FLAG;
	processingFlags[ CString("Histogram high cut fraction") ]	= RS_RENDER_FLAG;
	processingFlags[ CString("displayEnd") ]						= RS_TIMECHANGE_FLAG;
	processingFlags[ CString("displayStart") ]					= RS_TIMECHANGE_FLAG;

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
	MAPINTIT it = options.find( CString("Frequency scale") );
	if ( it == options.end() )
		return SCALE_LINEAR;
	else
		return (RS_SCALE_CHOICE) it->second; 
}

RS_GRAPH_CHOICE ParameterPack::GetGraphChoice()
{
	MAPINTIT it = options.find( CString("Graph choice") );
	if ( it == options.end() )
		return RS_GRAPH_NONE;
	else
		return (RS_GRAPH_CHOICE) it->second; 
}

RS_INTEGRATION_CHOICE ParameterPack::GetIntegrationChoice()
{
	MAPINTIT it = options.find( CString("Integration algorithm choice") );
	if ( it == options.end() )
		return INTEGRATION_NONE;
	else
		return (RS_INTEGRATION_CHOICE) it->second; 
}

RS_INTEGRATION_BY_FREQUENCY ParameterPack::GetIntegrationDependenceChoice()
{
	MAPINTIT it = options.find( CString("Integration frequency dependence") );
	if ( it == options.end() )
		return INTEGRATION_FLAT;
	else
		return (RS_INTEGRATION_BY_FREQUENCY) it->second; 
}

RS_DISPLAY_CHOICE ParameterPack::GetDisplayChoice()
{
	MAPINTIT it = options.find( CString("Display choice") );
	if ( it == options.end() )
		return RS_RESONANCE_SPECTRUM;
	else
		return (RS_DISPLAY_CHOICE) it->second; 
}

RS_SNAP_CHOICE ParameterPack::GetSnapChoice()
{
	MAPINTIT it = options.find( CString("Graph choice at snapshot") );
	if ( it == options.end() )
		return SNAP_NONE;
	else
		return (RS_SNAP_CHOICE) it->second; 
}

RS_ALGORITHM ParameterPack::GetAlgorithmChoice()
{
	MAPINTIT it = options.find( CString("Resonance algorithm choice") );
	if ( it == options.end() )
		return GAMMA_4POLE;
	else
		return (RS_ALGORITHM) it->second; 
}

RS_POLES_CHOICE ParameterPack::GetPolesChoice()
{
	MAPINTIT it = options.find( CString("Poles solution choice") );
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
			inputOption( CString("Display choice"), CString("Power spectrum") );
			inputOption( CString("Poles solution choice"), CString("Power only solution") );
			processHasChanged = TRUE;
		}
	} else {
		if ( dc == RS_POWER_SPECTRUM )
		{
			inputOption( CString("Display choice"), CString("Power spectrum") );
			inputOption( CString("Poles solution choice"), CString("Use one pole solution") );
			processHasChanged = TRUE;
		}
	}
	
	// Check Nyquist constraint
	double hf = GetHighFrequencyLimit();
	if ( hf > samplingRate / 2.0 )
	{
		parameters[ CString("High frequency limit") ] = samplingRate / 2.0;
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

bool ParameterPack::inputParam( CString key, double value )
{
	MAPDOUBLEIT it = parameters.find( key );
	if ( it == parameters.end() )
	{
		// no key: can't accept
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

bool ParameterPack::inputOption( CString key, CString value )
{
	MAPINTIT it = options.find( key );
	MAPSTRINGIT its = optionsByName.find( key );
	if ( it == options.end() )
	{
		// no key; can't accept
		return false;
	}
	else
	{
		its->second = value;

		// option key exists correctly; find the category and set in main map.
		if ( algorithmChoice.find( value ) != algorithmChoice.end() )
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
		} else if ( presetChoice.find( value ) != presetChoice.end() )
		{
			it->second = presetChoice[ value ];
			SetPresets();	//execute it
		} else if ( polesChoice.find( value ) != polesChoice.end() )
		{
			it->second = polesChoice[ value ];
		} else if ( scaleChoice.find( value ) != scaleChoice.end() )
		{
			it->second = scaleChoice[ value ];
		} else if ( strategyChoice.find( value ) != strategyChoice.end() )
		{
			it->second = strategyChoice[ value ];
			SetStrategy();	// execute it
		} else if ( snapChoice.find( value ) != snapChoice.end() )
		{
			it->second = snapChoice[ value ];
		} else
			return false;
		
		// set processing flags
		RS_PROCESSING_FLAG flag = (RS_PROCESSING_FLAG) processingFlags[ key ];
		if ( flag == RS_BITMAP_FLAG ) bitmapHasChanged = TRUE;
		else if ( flag == RS_PROCESS_FLAG ) processHasChanged = TRUE;
		else if ( flag == RS_RENDER_FLAG ) renderHasChanged = TRUE;
		else if ( flag == RS_GRAPH_FLAG ) graphChanged = TRUE;

	}
	return true;
}
void ParameterPack::SetPresets()
{
	
	int choice = options[ CString("Preset choice") ];
	if ( choice == RS_SPEECH )
	{
		inputParam( CString("High frequency limit"), 6000.0 );
		inputParam( CString( "Max bandwidth"), 600.0);

		inputParam( CString( "Integration time constant"), 0.002);
		inputParam( CString( "Noise floor"), -40.0);
		inputParam( CString( "Locality scale"), 3.0);
		inputParam( CString( "Bandwidth display scale"), 0.2);
		inputOption( CString( "Frequency scale"), CString("Logarithmic"));
		processHasChanged = TRUE;
	} else if ( choice == RS_MUSIC )
	{
		inputParam( CString( "Low frequency limit"), 20.0);
		inputParam( CString( "High frequency limit"), 16000.0);
		inputParam( CString( "Requested number of filters"), 400);
		inputParam( CString( "Min frequency spacing"), 4.0);
		inputParam( CString( "Max bandwidth"), 200.0);

		inputParam( CString( "Integration time constant"), 0.001);
		inputOption( CString( "Frequency scale"),  CString("Mel scale"));
		processHasChanged = TRUE;
		SetFrequencyScale();

	} else if ( choice == RS_BIRDSONG )
	{
		inputParam( CString( "Low frequency limit"), 1000.0);
		inputParam( CString( "High frequency limit"), 18000.0);
		inputParam( CString( "Max bandwidth"), 600.0);
		inputParam( CString( "Integration time constant"), 0.001);
		inputOption( CString( "Frequency scale"), CString("Log scale"));
		processHasChanged = TRUE;
	} else if ( choice == RS_BLOODPRESSURE )
	{
		inputParam( CString( "High frequency limit"), 10.0);
		inputParam( CString( "Low frequency limit"), 0.001);
		inputParam( CString( "Requested number of filters"), 100.0);
		inputParam( CString( "Min frequency spacing"), 0.001);
		inputParam( CString( "Integration time constant"), 1.0);
		processHasChanged = TRUE;
	} else if ( choice == RS_DOLPHIN )
	{
		inputParam( CString( "High frequency limit"), 200000.0);		// this will be limited by sampling rate, so safe
		inputParam( CString( "Max bandwidth"), 4000.0);				// about 10X human cohlear stiffness
		inputParam( CString( "Low frequency limit"), 100.0);
		inputParam( CString( "Requested number of filters"), 200.0);
		inputParam( CString( "Min frequency spacing"), 100.0);
		inputOption( CString( "Display choice"), CString("Reassignment spectrum"));
		inputParam( CString( "Integration time constant"), 4.0);
		inputParam( CString( "Bandwidth display scale"), 0.1);
		inputOption( CString( "Frequency scale"), CString("Logarithmic"));
		processHasChanged = TRUE;
	}
}

void ParameterPack::SetStrategy()
{
	
	int choice = options[ CString( "Strategy choice") ];
	if ( choice == RS_QUICKLOOK )
	{
		inputParam( CString( "Requested number of filters"), 50);
		inputParam( CString( "Display bitmap height"), 500);
		inputParam( CString( "Display bitmap width"), 500);
		processHasChanged = TRUE;
	} else if ( choice == RS_STANDARD )
	{
		inputParam( CString( "Display bitmap height"), 1000);
		inputParam( CString( "Display bitmap width"), 1000);
		processHasChanged = TRUE;

	} else if ( choice == RS_PRESENTATION )
	{
		inputParam( CString( "Display bitmap height"), 1000);
		inputParam( CString( "Display bitmap width"), 2000);
		processHasChanged = TRUE;
	}

}

CString ParameterPack::FindKeyByInt( MAPINT *pMap, int key )
{
	MAPINTIT it;
	for ( it = pMap->begin(); it != pMap->end(); it++ )
	{
		if ( it->second == key )
			return it->first;
	}
	return CString( "Not found" );
}

CString ParameterPack::FindKeyByDouble( MAPDOUBLE *pMap, double key )
{
	MAPDOUBLEIT it;
	for ( it = pMap->begin(); it != pMap->end(); it++ )
	{
		if ( it->second == key )
			return it->first;
	}
	return CString( "Not found" );
}

void ParameterPack::SetFrequencyScale()
{
	double h = parameters[ CString("Display bitmap height")];
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

CString ParameterPack::statusAsJSON()
{
	CString sOut;
	sOut = _T( " \"status\" : {");
	CString s;
	s.Format( _T("\"lowestFilterFrequency\" : %6.2f,\n\t"), outputParams.lowestFilterFrequency);
	sOut = sOut + s;
	s.Format( _T("\"highestFilterFrequency\" : %6.2f,\n\t"),	 outputParams.highestFilterFrequency);
	sOut = sOut + s;
	s.Format( _T("\"numberOfFilters\" : %d,\n\t"), (int) outputParams.numberOfFilters);
	sOut = sOut + s;
	s.Format( _T("\"processMemoryTime\" :%6.4f,\n\t"), outputParams.processMemoryTime);
	sOut = sOut + s;
	s.Format( _T("\"samplingInterval\" : %6.4f }"), outputParams.samplingInterval );
	sOut = sOut + s;
	return sOut;
}

CString ParameterPack::paramsAsJSON()
{
	CString sOut;
	sOut = " \"params\" : { ";
	MAPDOUBLEIT it;
	bool start = true;
	for ( it = parameters.begin(); it != parameters.end(); it++ )
	{
		if ( !start )
			sOut = sOut + _T(",\n\t");
		start = false;
		CString s = it->first;
		double d = it->second;
		CString f;
		f.Format( _T("\"%s\" : %f"), s, d );
		sOut = sOut + f;
	}
	sOut = sOut + _T(" }");
	return sOut;
}

CString ParameterPack::optionsAsJSON()
{
	CString sOut;
	sOut = " \"options\" : { ";
	MAPSTRINGIT it;
	bool start = true;
	for ( it = optionsByName.begin(); it != optionsByName.end(); it++ )
	{
		if ( !start )
			sOut = sOut + _T(",\n\t");
		start = false;
		CString s = it->first;
		CString s2 = it->second;
		CString f;
		f.Format( _T("\"%s\" : %s"), s, s2 );
		sOut = sOut + f;
	}
	sOut = sOut + _T(" }");
	return sOut;
}

BOOL ParameterPack::getJSONActiveLegend( CString *stringLegend )
{
	CString sOut;
	sOut = "\n{ ";
	sOut = sOut + statusAsJSON();
	sOut = sOut + _T(",\n ");
	sOut = sOut + paramsAsJSON();
	sOut = sOut + _T(",\n ");
	sOut = sOut + optionsAsJSON();
	sOut = sOut + _T(" }");
	*stringLegend = sOut;
	return TRUE;
}

void ParameterPack::SetLabels()
{
	logLabels[ CString("0.001") ] = 0.001;
	logLabels[ CString("0.0015") ] = 0.0015;
	logLabels[ CString("0.002") ] = 0.002;
	logLabels[ CString("0.003") ] = 0.003;
	logLabels[ CString("0.005") ] = 0.005;
	logLabels[ CString("0.01") ] = 0.01;
	logLabels[ CString("0.015") ] = 0.015;
	logLabels[ CString("0.02") ] = 0.02;
	logLabels[ CString("0.03") ] = 0.03;
	logLabels[ CString("0.05") ] = 0.05;
	logLabels[ CString("0.1") ] = 0.1;
	logLabels[ CString("0.15") ] = 0.15;
	logLabels[ CString("0.2") ] = 0.2;
	logLabels[ CString("0.3") ] = 0.3;
	logLabels[ CString("0.5") ] = 0.5;
	logLabels[ CString("1.0") ] = 1.0;
	logLabels[ CString("1.5") ] = 1.5;
	logLabels[ CString("2.0") ] = 2.0;
	logLabels[ CString("3.0") ] = 3.0;
	logLabels[ CString("5.0") ] = 5.0;
	logLabels[ CString("10.0") ] = 10.0;
	logLabels[ CString("15.0") ] = 15.0;
	logLabels[ CString("20.0") ] = 20.0;
	logLabels[ CString("30.0") ] = 30.0;
	logLabels[ CString("50.0") ] = 50.0;
	logLabels[ CString("100") ] = 100.0;
	logLabels[ CString("150") ] = 150.0;
	logLabels[ CString("200") ] = 200.0;
	logLabels[ CString("300") ] = 300.0;
	logLabels[ CString("500") ] = 500.0;
	logLabels[ CString("1000") ] = 1000.0;
	logLabels[ CString("1500") ] = 1500.0;
	logLabels[ CString("2000") ] = 2000.0;
	logLabels[ CString("3000") ] = 3000.0;
	logLabels[ CString("5000") ] = 5000.0;
	logLabels[ CString("10 kHz") ] = 10000.0;
	logLabels[ CString("15 kHz") ] = 15000.0;
	logLabels[ CString("20 kHz") ] = 20000.0;
	logLabels[ CString("30 kHz") ] = 30000.0;
	logLabels[ CString("50 kHz") ] = 50000.0;
	logLabels[ CString("100 kHz") ] = 100000.0;
	logLabels[ CString("150 kHz") ] = 150000.0;
	logLabels[ CString("200 kHz") ] = 200000.0;
	logLabels[ CString("300 kHz") ] = 300000.0;
	logLabels[ CString("500 kHz") ] = 500000.0;
	logLabels[ CString("1.0 MHz") ] = 1000000.0;

	// for equal tempered scale
	melLabels[ CString("C0") ] = 16.35;
	melLabels[ CString("A0") ] = 27.50;
	melLabels[ CString("A1 55") ] = 55.0;
	melLabels[ CString("A2 110") ] = 110;
	melLabels[ CString("A3 220") ] = 220.0;
	melLabels[ CString("A4 440") ] = 440.0;
	melLabels[ CString("A5 880") ] = 880.0;
	melLabels[ CString("A6 1760") ] = 1760.0;
	melLabels[ CString("A7") ] = 3520.0;
	melLabels[ CString("A8") ] = 7040.0;
	melLabels[ CString("10,000") ] = 10000.0;
	melLabels[ CString("15,000") ] = 15000.0;
	melLabels[ CString("20,000") ] = 20000.0;
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

