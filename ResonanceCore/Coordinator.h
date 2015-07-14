
// Coordinator.h : interface of the CCoordinator class
//

#pragma once
#include "Audio.h"
#include "ResonanceCoreProcess.h"
#include "DisplayBitmap.h"
#include "ParameterPack.h"
#include "Canvas.h"
#include "Logger.h"



enum SliceMode {TIME, ENERGY, FREQUENCY, INDICATION, SPECTRUM, SNAPSHOT};

typedef struct IMAGELEGEND {
	double startTime;
	double endTime;
	double samplingRate;
	double lowFrequency;
	double highFrequency;
	BOOL isLog;
	double valueRangeLow;
	double valueRangeHigh;
	CString imageType;
	CString sourceFileName;		// should be compatible with c# strings
} imageLegend;

class Coordinator
{
public:
	Coordinator( CString aWorkingPath );
	virtual ~Coordinator();

// Operations
public:
	// Interface
	BOOL OnOpenDocument(LPCTSTR  lpszPathName);
	void OnCloseDocument();
	BOOL getJSONActiveLegend( const char **stringLegend );
	BOOL inputOption( CString key, CString option );
	BOOL inputParam( CString key, double value );

// Implementation
public:

private:
	AudioSource		*pTheAudioSource;
	ResonanceStudioProcess *pTheResonanceStudioProcess;
	DisplayBitmap	*pTheDisplayBitmap;
	ParameterPack *pTheParameterPack;
	Canvas *pTheCanvas;
	Logger *pTheLogger;

private:
	CString workingPathName;
	CString audioPathName;
	double currentWorkStartTime;
	double currentWorkEndTime;
	double currentDisplayStartTime;
	double currentDisplayEndTime;
	double endOfFileTime;			// set when known.
	double samplingRate;
	double samplingInterval;

	IWICBitmap* ptheBitmap;

	double displayDistributionLowLimit;
	double displayDistributionHighLimit;

	IMAGELEGEND mainImageLegend;

private:
	// Graphing
	//void checkGraphingOptions();
	//void showSnapshot();

	// View interface
public:
	double	getCurrentDisplayStartTime() { return currentDisplayStartTime; };
	double	getCurrentDisplayEndTime() { return currentDisplayEndTime; };
	double	getSamplingRate() { return pTheAudioSource->GetPCMSamplingRate(); };
	CString getSignalPathName() { return audioPathName; };
	IMAGELEGEND* getImageLegend();	// Will replace above

	BOOL 	fillCanvas( double requestStartTime, double requestEndTime, double *actualDuration, BOOL *pEndOfStream );
	void		getSignalStroke( double time0, double time1, double rangeMin, double rangeMax, double *psignalWidth, double *psignalEndValue );
	void onSaveDisplayAs( CString pathname ) { pTheDisplayBitmap->onSaveDisplayAs( pathname ); };
	BOOL getSignalRange( double time0, double time1, double *rangeMin, double *rangeMax ) \
			{ return pTheCanvas->getSignalRange( time0, time1, rangeMin, rangeMax ); };

	BOOL processAndGetWICBitmap( IWICBitmap** ppWICBitmap );		// Main connection to draw, drives processing
	ParameterPack* getParameterPack() { return pTheParameterPack; };

//	BOOL		SetCurrentViewTime( double time1 );
	FVECTOR *getSpectrumAtTime( double time ) {return pTheCanvas->GetSpectrumAt( time ); };
	double	getCanvasMax() {return pTheCanvas->GetCanvasBufferMax(); };
	double	getCanvasMin() {return pTheCanvas->GetCanvasBufferMin(); };
	void showSlice( double frequency, double time, SliceMode mode );

	//(Future) Parameters
	int startRampTime;
	int decimation;			// for display or processing or both? To save time, space?

};

