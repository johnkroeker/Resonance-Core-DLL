
// Canvas.h : interface of the Canvas class
//

#pragma once
#include "ResonanceCoreProcess.h"
#include <vector>

enum SamplingMode {UNITSAMPLING, DOWNSAMPLING};

// Canvas holds the spectrum data and signal over time. It is meant to be intermediate in size: possibly long compared to current display,
// i.e. 16/32 to 1, possibly short relative to indefinite signal file sizes (> millions to 1). It keeps the memory and operations finite and definite
// while allowing time resolution to be as high as possible.
class Canvas
{

public:
	Canvas::Canvas( size_t timeSize );
	Canvas::~Canvas();

// Interface
public:
	// Process methods, in this order of state
	void Initialize();
	BOOL SetParameters( double samplingInterval, size_t spectrumSize, double t1, double t2 );
	BOOL AddSignalAndSpectrum(double time, double signalVal, const FVECTOR *pVector, double thisMax, double thisMin );
	BOOL Normalize();
	void SetToFull() { fullOfData = TRUE; };

	// Access methods
	size_t Size() { return size; };
	double GetSignalAt( double time );
	double GetSignalMinAt( double time );
	double GetSignalMaxAt( double time );
	FVECTOR* GetSpectrumAt( double time );
	double GetSpectrumMaxAt( double time );
	double GetSpectrumMinAt( double time );
	SamplingMode GetMode() { return mode; };
	
	double GetCanvasSignalMax() { return canvasSignalMax; };
	double GetCanvasSignalMin() { return canvasSignalMin; };
	BOOL getSignalRange( double time0, double time1, double *rangeMin, double *rangeMax );
	double GetCanvasBufferMax() { return canvasBufferMax; };
	double GetCanvasBufferMin() { return canvasBufferMin; };
	double GetTime1Limit() { return time1Limit; };
	double GetTime2Limit() { return time2Limit; };
	void SetTime2Limit( double t ) { time2Limit = t; };
	BOOL GetDistributionDisplayLimits( double lowTolerance, double highTolerance, double *lowLimit, double *highLimit );

	DVECTOR *GetDistribution();

// The Windows MF signal subsystem uses LONGLONG counts of 100 nsec increments. That sets the table for sizes here.
// A max file size is therefore at sampling of .1 usec (100 nsec) or 10 MHz, for +9223372036854775807 samples.
// Our size will be much less: e.g. 32000 for size, so 32000 * 2147483647 (32bit LONG max LONG_MAX). Or about 47 bits.
private:
	size_t size;

	SamplingMode mode;
	double ourSamplingInterval;
	double time1Limit;
	double time2Limit;
	BOOL fullOfData;


	double signalRMSLevel;
	double canvasSignalMax;					// for current data
	double canvasSignalMin;					//
	double canvasBufferMin;
	double canvasBufferMax;
	DVECTOR distribution;

	vector<float> signalBuffer;
	vector<float> signalMin;				// Used in DOWNSAMPLING to capture range
	vector<float> signalMax;

	// float saves half the memory, and we shouldn't need the bits-- unless we have wierd dynamic ranges
	vector<FVECTOR> spectrumBuffer;			// Main spectrum
	vector<float> occupancy;
	vector<double> spectrumMax;				// max value per time
	vector<double> spectrumMin;				// min value per time

private:
	size_t IndexFromTime( double time );
};
