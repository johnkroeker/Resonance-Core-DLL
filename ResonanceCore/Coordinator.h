
// Coordinator.h : interface of the CCoordinator class
//


#pragma once
#include "Audio.h"
#include "ResonanceCoreProcess.h"
#include "DisplayBitmap.h"
#include "ParameterPack.h"
#include "Canvas.h"

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
	Coordinator();
	virtual ~Coordinator();

// Operations
public:
	BOOL OnOpenDocument(LPCTSTR  lpszPathName);
	void OnCloseDocument();

// Implementation
public:

private:
	AudioSource		*pTheAudioSource;
	ResonanceStudioProcess *ptheResonanceStudioProcess;
	DisplayBitmap	*ptheDisplayBitmap;
	ParameterPack *ptheParameterPack;
	Canvas *ptheCanvas;

private:
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
	void CheckGraphingOptions();
	void ShowSnapshot();

	// View interface
public:
	void Report( CString s );
	double	GetCurrentDisplayStartTime() { return currentDisplayStartTime; };
	double	GetCurrentDisplayEndTime() { return currentDisplayEndTime; };
	double	GetSamplingRate() { return pTheAudioSource->GetPCMSamplingRate(); };
	CString GetSignalPathName() { return audioPathName; };
	IMAGELEGEND* GetImageLegend();	// Will replace above

	BOOL 	FillCanvas( double requestStartTime, double requestEndTime, double *actualDuration, BOOL *pEndOfStream );
	void		GetSignalStroke( double time0, double time1, double rangeMin, double rangeMax, double *psignalWidth, double *psignalEndValue );
	void OnSaveDisplayAs( CString pathname ) { ptheDisplayBitmap->OnSaveDisplayAs( pathname ); };
	BOOL GetSignalRange( double time0, double time1, double *rangeMin, double *rangeMax ) \
			{ return ptheCanvas->GetSignalRange( time0, time1, rangeMin, rangeMax ); };

	BOOL ProcessAndGetWICBitmap( IWICBitmap** ppWICBitmap );		// Main connection to draw, drives processing
	ParameterPack* GetParameterPack() { return ptheParameterPack; };

//	BOOL		SetCurrentViewTime( double time1 );
	FVECTOR *GetSpectrumAtTime( double time ) {return ptheCanvas->GetSpectrumAt( time ); };
	double	GetCanvasMax() {return ptheCanvas->GetCanvasBufferMax(); };
	double	GetCanvasMin() {return ptheCanvas->GetCanvasBufferMin(); };
	void ShowSlice( double frequency, double time, SliceMode mode );

	//(Future) Parameters
	int startRampTime;
	int decimation;			// for display or processing or both? To save time, space?

};

