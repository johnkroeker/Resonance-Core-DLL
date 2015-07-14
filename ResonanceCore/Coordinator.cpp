
// Coordinator.cpp : implementation of the Coordinator class
//

#include "stdafx.h"
#include "Logger.h"
#include "Coordinator.h"
//#include "MainFrm.h"
//#include "ResonanceCore.h"
//#include "ResonanceStudioDoc.h"
//#include "ResonanceStudioGraph.h"
#include "Canvas.h"
#include <propkey.h>

// Publish ourselves
Coordinator* pGlobalTheCoordinator = nullptr;
ParameterPack* theParameterPack = nullptr;


// Coordinator construction/destruction
Coordinator::Coordinator( CString aWorkingPath )
{
	pGlobalTheCoordinator = this;

	audioPathName = "";
	pTheAudioSource = nullptr; 
	pTheCanvas = nullptr;
	pTheResonanceStudioProcess = nullptr;
	pTheDisplayBitmap = nullptr;
	pTheLogger = nullptr;

	currentWorkStartTime = 0.0;
	currentWorkEndTime = 0.0;
	currentDisplayStartTime = 0.0;
	currentDisplayEndTime = 0.0;
	endOfFileTime = 0.0;
	samplingRate = 0.0;
	samplingInterval = 0.0;
	ptheBitmap = nullptr;
	displayDistributionLowLimit = 0.0;
	displayDistributionHighLimit = 0.0;

	pTheParameterPack = new ParameterPack();
	theParameterPack = pTheParameterPack;
	workingPathName = aWorkingPath;
}

// 
BOOL Coordinator::OnOpenDocument(LPCTSTR  lpszPathName)
	{
	CString logPath;
	logPath = workingPathName + _T("\\logfile.txt");
	pTheLogger = new Logger( logPath );

	audioPathName = lpszPathName;
	pTheAudioSource = new AudioSource();
	if (!pTheAudioSource->Create(lpszPathName) )
	{
		pTheLogger->fatal( _T("Coordinator"), _T( "File open failed" ) );
		return FALSE;
	}
	
	// Valid after Create.
	samplingRate = pTheAudioSource->GetPCMSamplingRate();
	if ( samplingRate <= 0.00 )
	{
		pTheLogger->fatal( _T("Coordinator"), _T( "Invalid sampling rate from file" ) );
		return FALSE;
	}
	samplingInterval = 1.0 / samplingRate;
	pTheResonanceStudioProcess = new ResonanceStudioProcess( pTheAudioSource );
	pTheCanvas = new Canvas( 16000 );

	pTheDisplayBitmap = new DisplayBitmap( this );
	UINT bitmapWidth = (UINT) pTheParameterPack->GetDisplayBitmapWidth();
	UINT bitmapHeight = (UINT) pTheParameterPack->GetDisplayBitmapHeight();
	pTheDisplayBitmap->Initialize( bitmapHeight, bitmapWidth );

//	pTheParameterPack->Process( samplingRate );

	return TRUE;
	};

void Coordinator::OnCloseDocument()
{
	if ( pTheAudioSource )
	{
		pTheAudioSource->ReleaseReader();
		pTheAudioSource = nullptr;
	}
	if ( pTheDisplayBitmap )
	{
		delete pTheDisplayBitmap;
		pTheDisplayBitmap = nullptr;
	}
}

Coordinator::~Coordinator()
{
	OnCloseDocument();
	delete pTheAudioSource;
	delete pTheCanvas;
	delete pTheResonanceStudioProcess;
	delete pTheDisplayBitmap;
	delete pTheParameterPack;
	delete pTheLogger;
}


// Overall time structure and strategy:
//		1.
//		2. The current display works with an interval of time. This interval may be contained in the current data, or need recomputation.
//		3. We are currently highly constrained by compute time, and somewhat by memory consumption. So we declare a maximum interval of time.
//		4. Computation proceeds time-wise with a current state. So incremental computation that ADDS to work results up to a time is possible.
//
//		Given the above, the flow is as follows:
//			We have an interval of work w=[w1,w2], possibly empty.
//			Some UI requests a display interval. d=[d1,d2]
//				d2 will be the lesser of d2 and d1 + maxInterval.
//			If d is not strictly contained in w, we clear all computation state and memory and recompute.
//	*			Two odd cases arise: small change in w1, and small change in w2. Any decrease in w1 still requires recompute. But a small increase in w2 should not.
//	*				Let us allow incremental adds up to some tolerance, e.g. 150% of maxInterval. 
//				Recompute will start with an audio seek.
//

// View Interface

// Called by OnDraw. True if new bitmap
BOOL Coordinator::processAndGetWICBitmap(IWICBitmap** ppWICBitmap  )
{
	BOOL processHasChanged = FALSE;
	BOOL renderChanged = FALSE;
	BOOL bitmapChanged = FALSE;
	pTheParameterPack->Process( samplingRate, &processHasChanged, &renderChanged, &bitmapChanged  );

	CStringA JSON;
	pTheParameterPack->getJSONActiveLegend( &JSON );
	pTheLogger->debug( _T("Coordinator"), JSON.GetBuffer() );

	//checkGraphingOptions();
	//showSnapshot();

	// Collect conditions. Check for a parameter change
	BOOL timeChanged =		pTheParameterPack->GetAndClearTimeChangeFlag();

	double time0 =			pTheParameterPack->GetDisplayStart();
	double time1 =			pTheParameterPack->GetDisplayEnd();
	double rampTime = pTheResonanceStudioProcess->GetProcessMemoryTime();

	// Validate
	if (time0 < 0.0 || time0 >= time1 || time1 < 0.0 )
	{
		currentWorkStartTime = 0.0;
		currentWorkEndTime = 0.0;
		currentDisplayStartTime = 0.0;
		currentDisplayEndTime = 0.0;
		endOfFileTime = 0.0;
		pTheLogger->fatal( _T("Coordinator"), _T( "Illegal time request" ) );
		return FALSE;
	}

	// We could put the 50% tolerance here. Ignore requests past end time if EOF already.
	double rampedTime0 = max( 0, time0 - rampTime);

	BOOL timeLimitChange = rampedTime0 < currentWorkStartTime || ( time1 > currentWorkEndTime && endOfFileTime == 0.0 );
	
	// Get state of Canvas-- if display times contract, we may need to recompute for resolution.

	// Change and re-init the processes first, so that we have parameters from which to work. THIS should match calls with fillCanvas below, in order to properly init.
	if ( timeLimitChange || processHasChanged )
	{
		// Something is new. Must rebuild the process and its viewbuffer.
		delete pTheResonanceStudioProcess;
		pTheResonanceStudioProcess = new ResonanceStudioProcess( pTheAudioSource );
		if ( pTheResonanceStudioProcess == nullptr )
		{
			pTheLogger->fatal( _T("Coordinator"), CString("Failure to create ResonaceStudioProcess") );
			return FALSE;
		}		
		rampTime = pTheResonanceStudioProcess->GetProcessMemoryTime();
//		report( CString( "Creating process") );
	}


	// to add: AND UNITSAMPLING is the case (otherwise the resolution changes)
	if ( !timeLimitChange && timeChanged )
	{
		// Must have embedded time change. Will need to re-render, but workspace times don't change.
		currentDisplayStartTime = max( time0, currentWorkStartTime );
		currentDisplayEndTime = min( time1, currentWorkEndTime );
		renderChanged = TRUE;
	}

	if ( timeLimitChange || processHasChanged )
	{
		CString s;
		s.Format(_T("flags: bitmp %d time %d process %d"), bitmapChanged, timeLimitChange, processHasChanged);
		pTheLogger->info( _T("Coordinator"), CString( "Processing...")+s );

		// Something is new. Must rebuild the canvas
		currentWorkStartTime = rampedTime0;
		currentWorkEndTime = currentWorkStartTime;
		endOfFileTime = 0.0;

		double actualDuration = 0.0;
		BOOL endOfStream = FALSE;
		if ( !fillCanvas( currentWorkStartTime, time1, &actualDuration, &endOfStream ) )
		{
			pTheLogger->fatal( _T("Coordinator"), CString("Processing or file error") );
			return FALSE;
		}

		if ( endOfStream )
		{
			endOfFileTime = currentWorkStartTime + actualDuration;
		}

		if ( actualDuration == 0.0 )
		{
			pTheLogger->fatal( _T("Coordinator"), CString("No Data to display based on commands") );
			return FALSE;
		}
		double actualEndTime = currentWorkStartTime + actualDuration;
		currentWorkEndTime = actualEndTime;		// all done, register that.
		currentDisplayStartTime = time0;
		currentDisplayEndTime = currentWorkEndTime;
		s.Format( _T("...Processing done to %9.4f secs"), currentDisplayEndTime );
		pTheLogger->info( _T("Coordinator"), CString( s ) );
	}

	if ( bitmapChanged )
	{
		UINT bitmapWidth = (UINT) pTheParameterPack->GetDisplayBitmapWidth();
		UINT bitmapHeight = (UINT) pTheParameterPack->GetDisplayBitmapHeight();
		pTheDisplayBitmap->Initialize( bitmapHeight, bitmapWidth );
	}

	// Now render if necessary
	if ( bitmapChanged || timeLimitChange || processHasChanged || renderChanged )
	{
		double lowTolerance = pTheParameterPack->GetDisplayDistributionLow(); 
		double highTolerance = pTheParameterPack->GetDisplayDistributionHigh(); 
		if ( !pTheCanvas->GetDistributionDisplayLimits( lowTolerance, highTolerance, &displayDistributionLowLimit, 
			&displayDistributionHighLimit ) )
		{
		pTheLogger->fatal( _T("Coordinator"), CString("Error finding display limits in canvas") );
			return FALSE;
		}

		//double enhanceValue = pTheParameterPack->GetDisplayPeakEnhanceValue(); 
		*ppWICBitmap = pTheDisplayBitmap->RenderSpectrumBitmap( displayDistributionLowLimit, displayDistributionHighLimit, 
			0.0, currentDisplayStartTime, currentDisplayEndTime );

		// Save the jpeg file for pickup
		CString imagePath;
		imagePath = workingPathName + _T("\\spectrum.jpg");

		if ( !pTheDisplayBitmap->onSaveDisplayAs( imagePath ) )
		{
			pTheLogger->fatal( _T("Coordinator"), _T("Could not save jpeg file") );
			return FALSE;
		}
		return TRUE;
	}
	
	// If the interval falls in our data buffer, no work to do
	return FALSE;
}
/*
void Coordinator::checkGraphingOptions()
{
	RS_GRAPH_CHOICE gc = pTheParameterPack->GetGraphChoice();
	CString graphTitle;
	CString graphName = pTheParameterPack->GetGraphName( gc );

	if ( pTheResonanceStudioProcess == nullptr )
	{
		pTheResonanceStudioProcess = new ResonanceStudioProcess( pTheAudioSource );
	}		

	if ( gc == GRAPH_CENTERS )
	{
		DVECTOR *freqs = pTheResonanceStudioProcess->GetCenterFrequencies();
		theGraphWindow->ShowSingleLine( CString("Center frequencies of the complex filter bank"), CString("Filters"), CString("Frequency"), freqs );
	}
	else if ( gc == GRAPH_BANDWIDTH )
	{
		DVECTOR *bandwidths = pTheResonanceStudioProcess->GetFilterBandwidths();
		DVECTOR *freqs = pTheResonanceStudioProcess->GetCenterFrequencies();
		theGraphWindow->ShowXvsY( CString("Bandwidths of the complex filter bank"), CString("Filter frequency"), CString("Bandwidth"), 
			freqs, bandwidths );
	}
	else if ( gc == GRAPH_INTEGRATION_ENVELOPE )
	{
		double tau = pTheParameterPack->GetIntegrationTau();
		RS_INTEGRATION_CHOICE ic = pTheParameterPack->GetIntegrationChoice();
		double order = 1.0;
		if ( ic == COHERENCE_ORDER_TWO ) order = 2.0;
		if ( ic == INTEGRATION_NONE ) order = 0.0;
		graphTitle.Format( _T("Envelope of the Coherence integrator for %s"), pTheParameterPack->GetIntegrationName( ic ) );
		theGraphWindow->ShowCoherenceEnvelope(graphTitle, CString("Time in msecs"), CString("Relative amplitude"), tau, order );
	}
	else if ( gc == GRAPH_FILTER_ENVELOPE )
	{
		RS_ALGORITHM ic = pTheParameterPack->GetAlgorithmChoice();
		double order = 1.0;
		if ( ic == GAMMA_4POLE ) order = 4.0;
		DVECTOR *pbw = pTheResonanceStudioProcess->GetFilterBandwidths();
		graphTitle.Format( _T("Filter time envelopes at min and max bandwidth specified for %s"), pTheParameterPack->GetAlgorithmName( ic ) );
		theGraphWindow->ShowFilterEnvelopes( graphTitle, CString("Time in msecs"), CString("Relative amplitude"),
			pbw->front(), pbw->back(), order );
	}
	else if ( gc == GRAPH_LEVEL_DISTRIBUTION )
	{
		DVECTOR *pDistribution = pTheCanvas->GetDistribution();
		theGraphWindow->ShowSingleLine( CString("Current level distribution"), CString("bin"), CString("Density"), pDistribution );
	}

}
*/
// Returns actual end time. Check for shortness on EOF
// Always process sample to keep signal processing in sync with current audio
BOOL Coordinator::fillCanvas(double requestedStartTime, double requestedEndTime, double *pActualDuration, BOOL *pEndOfStream )
{
	if ( !pTheAudioSource )
	{
		pTheLogger->fatal( _T("Coordinator"), CString( "Audio object not created" ) );
		return FALSE;
	}

	// Check both ends of requests before we begin-- we need to know so that the mode is correct.
	*pActualDuration = 0.0;
	BOOL endOfStream = FALSE;
	if ( !pTheAudioSource->Seek( requestedStartTime, &endOfStream ) )
	{
		pTheLogger->fatal( _T("Coordinator"), CString( "Seek to requested start fails" ) );
		return FALSE;
	}
	requestedEndTime = min( requestedEndTime, pTheAudioSource->GetDurationSeconds() );
	BOOL first = TRUE;
	for ( double time = requestedStartTime; time <= requestedEndTime; time += samplingInterval )	
	{
		double signalVal1;
		double signalVal2;
		if ( !pTheAudioSource->GetNextPCMSample( &signalVal1, &signalVal2, pEndOfStream ) )
		{
			pTheLogger->fatal( _T("Coordinator"), CString("Failed to load next data sample") );
			return FALSE;
		}
		if ( *pEndOfStream )
		{
			break;
		}
		if ( !pTheResonanceStudioProcess->Process(signalVal1, time ) )
		{
			pTheLogger->fatal( _T("Coordinator"), CString("Process fails") );
			return FALSE;
		}
		if (first )
			{
			pTheCanvas->SetParameters( samplingInterval, (pTheResonanceStudioProcess->GetDisplaySpectrum())->size(), requestedStartTime, requestedEndTime );
			first = FALSE;
		}
		pTheCanvas->AddSignalAndSpectrum( time, signalVal1, pTheResonanceStudioProcess->GetDisplaySpectrum(),
			pTheResonanceStudioProcess->GetSpectrumMax(), pTheResonanceStudioProcess->GetSpectrumMin() );

		*pActualDuration += samplingInterval;
	}
	// Possibly correct the upper limit in Canvas, due to end of file.
	pTheCanvas->SetTime2Limit( *pActualDuration + requestedStartTime );
	pTheCanvas->Normalize();
	return TRUE;
}

// returns value in [0.0, 1.0] -- scaled to min/max as given.
void Coordinator::getSignalStroke( double time0, double time1, double minRange, double maxRange, double *pSignalWidth, double *pSignalEndValue )
{
	*pSignalWidth = 0.0;
	*pSignalEndValue = 0.0;
	if ( time0 < currentWorkStartTime || time0 > currentWorkEndTime ||
		 time1 < currentWorkStartTime || time1 > currentWorkEndTime)
		return;

	double range = maxRange - minRange;
	if ( range <= 0.0 )
		return;

	// Get the range for the duration of this stroke.
	double minVal = 0.0;
	double maxVal = 0.0;
	pTheCanvas->getSignalRange( time0, time1, &minVal, &maxVal );

	*pSignalWidth = abs(maxVal - minVal) / range;
	*pSignalEndValue = ( (maxVal + minVal) / 2.0 - minRange)/ range;
}
/*
void Coordinator::showSlice( double frequency, double time, SliceMode mode )
{
	double highFrequencyLimit = pTheParameterPack->GetFilterHigh();
	double lowFrequencyLimit = pTheParameterPack->GetFilterLow();
	CString graphTitle;

	if ( mode == SPECTRUM )
	{
		CString alg = pTheParameterPack->GetAlgorithmName( pTheParameterPack->GetAlgorithmChoice() );
		graphTitle.Format( _T("%s Spectrum at %-6.4f secs"), alg, time );
		theGraphWindow->ShowSpectrumSlice( graphTitle, CString( "Frequency" ), CString("Information density (bits)"), 
					pTheCanvas->GetSpectrumAt( time ), lowFrequencyLimit, highFrequencyLimit);
	}
		if ( mode == SNAPSHOT )
	{
	}

}

void Coordinator::showSnapshot()
{
	double time = pTheParameterPack->GetSnapshotTime();
	
	double highFrequencyLimit = pTheParameterPack->GetFilterHigh();

	RS_SNAP_CHOICE sc = pTheParameterPack->GetSnapChoice();
	CString filterChoice = pTheParameterPack->GetAlgorithmName( pTheParameterPack->GetAlgorithmChoice() );
	CString graphTitle;

	if ( time <= 0.0 ) return;

	if ( sc == SNAP_NONE )
	{
	//graphTitle.Format( _T("Indicated frequencies v. centers at %-6.4f secs for %s"), time, filterChoice );
	//	theGraphWindow->ShowXvsY( graphTitle, CString("Filter Frequency"), CString("Indicated frequency"),
	//		&pTheResonanceStudioProcess->snapEstimatedFrequencies,  
	//		pTheResonanceStudioProcess->GetCenterFrequencies() );
	}
	else if ( sc == SNAP_ENERGY )
	{
		graphTitle.Format( _T("Energy slice at %-6.4f secs for %s"), time, filterChoice );
		theGraphWindow->ShowX_vs_dBY( graphTitle, CString("Filter Frequency"), CString("Power in dB"),
			 pTheResonanceStudioProcess->GetCenterFrequencies(), &pTheResonanceStudioProcess->snapEnergy );
	}
	else if ( sc == SNAP_INDICATION )
	{
		graphTitle.Format( _T("Indicated Frequency at Intensity %-6.4f secs for %s"), time, filterChoice );

		theGraphWindow->ShowIndicationSlice( graphTitle, CString("Filter Frequency"), CString("Intensity in bits\n"), 
			&pTheResonanceStudioProcess->snapEstimatedFrequencies0,  
			&pTheResonanceStudioProcess->snapEstimatedFrequencies1,  
			pTheResonanceStudioProcess->GetCenterFrequencies(), &pTheResonanceStudioProcess->snapIntensity );
	}
	else if ( sc == SNAP_POLES )
	{
		graphTitle.Format( _T("Poles v. filter poles at %-6.4f secs for %s"), time, filterChoice );
		theGraphWindow->ShowSnapPoles( graphTitle, CString("Real"), CString("Imaginary"),pTheResonanceStudioProcess->GetFilterPoles(), 
			&(pTheResonanceStudioProcess->snapPoles0), &(pTheResonanceStudioProcess->snapPoles1) );
	}
	else if ( sc == SNAP_DEVIATION )
	{
		graphTitle.Format( _T("Frequency deviation at %-6.4f secs for %s"), time, filterChoice);
		theGraphWindow->ShowXvsTwoY( graphTitle,  CString("Filter center frequency"), CString("Estimated frequency"),
			pTheResonanceStudioProcess->GetCenterFrequencies(), pTheResonanceStudioProcess->GetCenterFrequencies(), 
			&(pTheResonanceStudioProcess->snapEstimatedFrequencies1) );
	}
	else if ( sc == SNAP_ENTROPY )
	{
		graphTitle.Format( _T("Entropy slice at %-6.4f secs for %s"), time, filterChoice );

		theGraphWindow->ShowEntropySlice( graphTitle, CString("Filter center frequency"), CString("Entropy in log scale"),
			&(pTheResonanceStudioProcess->snapH0), 
			pTheResonanceStudioProcess->GetCenterFrequencies(), &(pTheResonanceStudioProcess->snapDisplay) );
	}
	else if ( sc == SNAP_CONVERGENCE )
	{
		graphTitle.Format( _T("Convergence and localIntensity at %-6.4f secs for %s"), time, filterChoice );
		theGraphWindow->ShowConvergence( graphTitle, CString("Filter center frequency"), CString("Combined scale"),
			&(pTheResonanceStudioProcess->snapConvergence), &(pTheResonanceStudioProcess->snapLocalIntensity), 
			pTheResonanceStudioProcess->GetCenterFrequencies(), highFrequencyLimit );
	}
	else if ( sc == SNAP_TIMESHIFT )
	{
		graphTitle.Format( _T("Re-assigment time shift at %-6.4f secs for %s"), time, filterChoice );
		theGraphWindow->ShowXvsY( graphTitle, CString("Indicated frequency"), CString("Delta time in ms"),
			&(pTheResonanceStudioProcess->snapEstimatedFrequencies0), &(pTheResonanceStudioProcess->snapTimeShift)  );
	}
}
*/