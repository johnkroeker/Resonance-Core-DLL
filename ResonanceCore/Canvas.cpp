
// Coordinator.cpp : implementation of the Coordinator class
//

#include "stdafx.h"
#include "Logger.h"
#include "Canvas.h"
#include "ResonanceCore.h"

Canvas::Canvas( size_t aSize )
{
	size = aSize;
	Initialize();
}

void Canvas::Initialize()
{
	canvasSignalMax = -DBL_MAX;
	canvasSignalMin = DBL_MAX;
	canvasBufferMax = -DBL_MAX;
	canvasBufferMin = DBL_MAX;

	signalBuffer.clear();
	signalBuffer.assign( size, 0.0f );
	signalMin.clear();
	signalMin.assign( size, FLT_MAX );
	signalMax.clear();
	signalMax.assign( size, -FLT_MAX );

	// Not pre-allocated
	spectrumBuffer.clear();
	occupancy.clear();
	occupancy.assign( size, 0.0 );
	spectrumMax.clear();
	spectrumMax.assign( size, -DBL_MAX );
	spectrumMin.clear();
	spectrumMin.assign( size, DBL_MAX );
	fullOfData = FALSE;
	time1Limit = 0.0;
	time2Limit= 0.0;
	mode = UNITSAMPLING;
	ourSamplingInterval = 0.0;
	distribution.clear();
}

Canvas::~Canvas()
{
// All stl vectors go out of scope here and should take care of themselves
}


// We might like the caller to know 1) success/fail, 2) which mode we are in, and 3) whether to process
BOOL Canvas::SetParameters( double samplingInterval, size_t spectrumSize, double t1, double t2  )
{
	// WE need to know: empty or full right now? If full, what are current limits and mode.
	if ( fullOfData )
	{
		// Check to see if we can continue with no processing.
		// But then what state will we be in??
		// Return or fall through
		fullOfData = FALSE;
	}

	// We are going to process and re-fill
	Initialize();
	ourSamplingInterval = samplingInterval;

	double requestedSize = ( t2 - t1 ) / samplingInterval;

	// Check for cosmic limits, assuming size_t is LONG_MAX limited.
	if ( requestedSize / ( (double) LONG_MAX ) > (double) size )
	{
		// Log this!
		return FALSE;
	}
	mode = requestedSize > size ? DOWNSAMPLING : UNITSAMPLING;
	time1Limit = t1;
	time2Limit = t2;
	fullOfData = FALSE;

	// initialize big buffer
	for ( size_t i = 0; i < size; i++ )
	{
		FVECTOR tmp;
		tmp.assign( spectrumSize, 0.0f );
		spectrumBuffer.push_back( tmp );
	}
	return TRUE;
}

size_t Canvas::IndexFromTime( double time )
{
	if ( mode == UNITSAMPLING )
	{
		return (size_t) ( ( time - time1Limit ) / ourSamplingInterval );
	} else
	{
		return (size_t) ( (  (double) size ) * ( time - time1Limit ) / (time2Limit - time1Limit ) );
	}
}

// If/when we do a client/server split we may split off the signal. 

// Supply absolute time, signal, and processing results for that time.
// ASSUMES that the vector stream is all of the same size.
BOOL Canvas::AddSignalAndSpectrum(double time, double dSignalVal, const FVECTOR *pVector, double thisMax, double thisMin )
{
	size_t currentIndex = IndexFromTime( time );
	if ( currentIndex < 0 || currentIndex >= size ) return FALSE;
	float signalVal = (float) dSignalVal;

	canvasSignalMax = max( canvasSignalMax, signalVal );
	canvasSignalMin = min( canvasSignalMin, signalVal );

	if ( mode == UNITSAMPLING )
	{
		signalBuffer[ currentIndex ] = signalVal;
		signalMin[ currentIndex ] = signalVal;
		signalMax[ currentIndex ] = signalVal;

		spectrumBuffer[currentIndex] = *pVector;
		occupancy[ currentIndex ] = 1.0;
		spectrumMax[ currentIndex ] = thisMax;
		spectrumMin[ currentIndex ] = thisMin;
	} else
	{
		signalBuffer[ currentIndex ] += signalVal;		// this keeps the sum
		signalMin[ currentIndex ] = min( signalMin[ currentIndex ], signalVal );
		signalMax[ currentIndex ] = max( signalMax[ currentIndex ], signalVal );
		
		if ( occupancy[currentIndex] == 0.0 )
		{
			// first one in created and noted.
			spectrumBuffer[ currentIndex ] = *pVector;
			occupancy[ currentIndex ] = 1.0;
		} else
		{
			if ( spectrumBuffer[ currentIndex ].size() != pVector->size() ) return FALSE;

			// subsequent ones get added and counted
			for ( size_t i = 0; i < pVector->size(); i++ )
			{
				spectrumBuffer[ currentIndex][i] += (*pVector)[i];
			}
			occupancy[ currentIndex ]++;
		}
		spectrumMin[ currentIndex ] = min( spectrumMin[ currentIndex ], thisMin );
		spectrumMax[ currentIndex ] = max( spectrumMax[ currentIndex ], thisMax );
	}
	canvasBufferMin = min( spectrumMin[ currentIndex ], canvasBufferMin );
	canvasBufferMax = max( spectrumMax[ currentIndex ], canvasBufferMax );
	return TRUE;
}

// compute level distribution
DVECTOR* Canvas::GetDistribution()
{
	const size_t N = 100;
	distribution.assign( N, 0.0 );
	double ourMax = canvasBufferMax > 0.0 ? log10( canvasBufferMax ) : 1.0;
	double ourMin = canvasBufferMin > 0.0 ? log10( canvasBufferMin ) : 0.0;
	for ( size_t i = 0; i < spectrumBuffer.size(); i++ )
	{
		for ( size_t j = 0; j < spectrumBuffer[i].size(); j++ )
		{
			double x = spectrumBuffer[i][j];
			
			// Ignore the zeros
			if ( x <= 0.0 ) continue;
			x = log10(x);

			double r = ( x - ourMin ) / ( ourMax - ourMin );
			int level = (int) ( ( (double) N ) * r);
			level = max( 0, level );
			level = min ( (int) distribution.size()-1, level );
			distribution[ level ]++;
		}
	}
	return &distribution;
}

// Get e.g. the 5% and 95% points for .05 tolerance
BOOL Canvas::GetDistributionDisplayLimits( double lowTolerance, double highTolerance, double *lowLimit, double *highLimit )
{
	*lowLimit = 0.0;
	*highLimit = 1.0;
	if ( distribution.size() == 0 )
		GetDistribution();
	if ( lowTolerance < 0.0 || lowTolerance >= 0.5 )
		return FALSE;
	if ( highTolerance < 0.0 || highTolerance >= 0.5 )
		return FALSE;

	// sum up for total count
	double sum = 0.0;
	for ( size_t i = 0; i < distribution.size(); i++ )
		sum += distribution[i];
	if ( sum <= 0.0 )
		return FALSE;

	// scan for limits
	double lowSum = lowTolerance * sum;
	double highSum = ( 1.0 - highTolerance ) * sum;
	size_t lowIndex = 0;
	size_t highIndex = 0;
	sum = 0.0;
	for ( size_t i = 0; i < distribution.size(); i++ )
	{
		sum += distribution[i];
		if ( sum >= lowSum && lowIndex == 0 )
			lowIndex = i;
		if ( sum >= highSum && highIndex == 0 )
			highIndex = i;
	}

	// Return proportionate limits in [0,1] of course.
	*lowLimit =  (double) lowIndex / (double ) distribution.size();
	*highLimit = (double) highIndex / ( double) distribution.size();
	return TRUE;
	}


// Get range directly from signalBuffer. Be very careful with exceptions.
BOOL Canvas::GetSignalRange( double time0, double time1, double *rangeMin, double *rangeMax )
{
	*rangeMin = 0.0;
	*rangeMax = 0.0;

	size_t index0 = IndexFromTime( time0 );
	size_t index1 = IndexFromTime( time1 );
	if ( index0 < 0 || index1 < 0 )
		return FALSE;
	index0 = min( index0, signalBuffer.size()-1 );
	index1 = min( index1, signalBuffer.size()-1 );

	double minVal = signalMin[ index0 ];
	double maxVal = signalMax[ index0 ];
	for ( size_t i = index0; i < index1; i++ )
	{
		minVal = min( minVal, signalMin[i] );
		maxVal = max( maxVal, signalMax[i] );
	}
	*rangeMin = minVal;
	*rangeMax = maxVal;
	return TRUE;
}

FVECTOR* Canvas::GetSpectrumAt( double time )
{ 
	if( time < time1Limit || time > time2Limit ) return nullptr;
	if ( ourSamplingInterval == 0.0 ) return nullptr;

	size_t index = IndexFromTime( time );
	return index < size ? &spectrumBuffer[index] : nullptr;
};

double Canvas::GetSignalMinAt( double time )
{
	if( time < time1Limit || time > time2Limit ) return 0.0;
	if ( ourSamplingInterval == 0.0 ) return 0.0;

	size_t index = IndexFromTime( time );
	return index < size ? signalMin[index] : 0.0;
}

double Canvas::GetSignalMaxAt( double time )
{
	if( time < time1Limit || time > time2Limit ) return 0.0;
	if ( ourSamplingInterval == 0.0 ) return 0.0;

	size_t index = IndexFromTime( time );
	return index < size ? signalMax[index] : 0.0;
}

double Canvas::GetSignalAt( double time )
{
	if( time < time1Limit || time > time2Limit ) return 0.0;
	if ( ourSamplingInterval == 0.0 ) return 0.0;

	size_t index = IndexFromTime( time );
	return index < size ? signalBuffer[index] : 0.0;
}

double Canvas::GetSpectrumMaxAt( double time )
{
	if( time < time1Limit || time > time2Limit ) return 0.0;
	if ( ourSamplingInterval == 0.0 ) return 0.0;

	size_t index = IndexFromTime( time );
	return index < size ? spectrumMax[index] : 0.0;
}

double Canvas::GetSpectrumMinAt( double time )
{
	if( time < time1Limit || time > time2Limit ) return 0.0;
	if ( ourSamplingInterval == 0.0 ) return 0.0;

	size_t index = IndexFromTime( time );
	return index < size ? spectrumMin[index] : 0.0;
}

// Call when all values are in
BOOL Canvas::Normalize()
{
	if( mode == UNITSAMPLING ) return TRUE;

	for ( size_t i = 0; i < size; i++ )
	{
		float n = occupancy[i];
		if ( n > 1 )
		{
			for ( size_t j = 0; j < spectrumBuffer[i].size(); j++ )
			{
				spectrumBuffer[i][j] /= n;
			}
			signalBuffer[i] /= n;
		}
	}
	return TRUE;
}