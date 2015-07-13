// SignalProcesses.cpp : Definitions for the signal processing objects
//

#include "stdafx.h"
#include "ResonanceCore.h"
#include "SignalProcesses.h"
#include <math.h>

// triangular fuzzy logic function
static double fuzzyLocality( double x, double center, double scale )
{
	if ( scale <= 0.0 ) return 0.0;
	double d = abs( x - center );
	return max( 0.0, 1.0 - d / scale );
}

FilterBankArray::FilterBankArray()
{
	lowFrequency = 0.0;
	highFrequency= 0.0;
	out.clear();
	designFrequencies.clear();
	designBandwidths.clear();
	filterPoles.clear();
}

FilterBankArray::~FilterBankArray()
{
	out.clear();
	designFrequencies.clear();
	designBandwidths.clear();
	filterPoles.clear();
}

Gammatone1PoleArray::Gammatone1PoleArray() : FilterBankArray()
{
}

// Returns count of filters
long Gammatone1PoleArray::Initialize( double samplingInterval, double maxBandwidth,  double lowFreqLimit, double highFreqLimit,
									double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime )
{
	lowFrequency = lowFreqLimit;
	highFrequency = highFreqLimit;

	double pi = 3.14159265359;
	double twoPi = 2.0 * pi;
	double e = 2.718281828459045;
	CD ii( 0.0, 1.0 );

	// Return memory time = 1/ lowest bandwidth
	*pTheMemoryTime = 1.0 / ( twoPi * lowFrequency );

	// Auditory threshold at 1000 Hz re Praat is P0 = 1.0e-5 Pa, which Praat assumes by formula to be x=1, relative to full-scale 16-bits = 32768.
//	double inputScale = 1.0 / 32768.0;
//	double P0 = 1.0e-5;
	a1.clear();
	designFrequencies.clear();
	designBandwidths.clear();

	// Calculate the frequencies-- TODO should report how many somewhere
	for ( double f = lowFrequency; f <= highFrequency; f += max( minSpacing, f * proportionalSpacing ) )
	{
		designFrequencies.push_back( f );
		
		// Check bandwidths limits and set it. THIS SHOULD BE EVEN MORE AGRESSIVE for this one pole case. About f.center / 4.
		double bandwidth = min( 0.25 * f, maxBandwidth );
		designBandwidths.push_back( bandwidth );

		// The norm depends on tau
		double tau = 1.0 / (twoPi * bandwidth);
//		double ntau = dt / tau;
		// Series sum of the envelope is not the same as the full series-sum of the decaying sine wave, which should be about 0.
//		double seriesSum = 1.0;
//		double norm = inputScale / (1.0 * seriesSum);

		// Calculate. See Mathematica derivation.
		// a1 = Exp[-BT - i omega T ]
		CD k =-2.0 * pi * ii * f * samplingInterval - samplingInterval / tau;
		CD ca1 = pow(e, k);
	
		// Add to coefficient vector
		a1.push_back( ca1 );
	}
	Size = a1.size();
	
	// Build a vector of the filter poles
	filterPoles.clear();
	for ( size_t i = 0; i < designFrequencies.size(); i++ )
	{
	
		double r = exp( -pi * designBandwidths[i] * samplingInterval );
		double real = cos( samplingInterval * 2.0 * pi * designFrequencies[i] );
		double imag = -sin( samplingInterval * 2.0 * pi * designFrequencies[i] );
		CD pole ( r * real, r * imag );
		filterPoles.push_back( pole );
	}

	// allocate and zero out the memories
	y1.assign( Size, CD(0.0,0.0) );

	out.assign( Size, CD(0.0,0.0) );
	return Size;
}

Gammatone1PoleArray::~Gammatone1PoleArray()
{
}

BOOL Gammatone1PoleArray::Process( double audioIn)
{
	// Real part is real input
	CD in(audioIn, 0.0 );

	for ( size_t i = 0; i < Size; i++ )
	{
		out[i] = a1[i] * y1[i] + in;
	}
	y1 = out;
	return TRUE;
}
//----------------------------------------------------------------------------------------------------------------

Gammatone4PoleArray::Gammatone4PoleArray() : FilterBankArray()
{
}

// Returns count of filters
/*
The new rule for complex (Hilbertian) transformation is that the negative pole should be quite definitely blocked. For the 4 pole filter here, the
fall off is about -12 dB per f0, or -24 dB to the negative pole -- which should reduce it to less than 1%. Other filters would have different
characteristics. 

This is the justification for making bandwwidth no more than f0.
*/
long Gammatone4PoleArray::Initialize( double samplingInterval, double maxBandwidth, double lowFreqLimit, double highFreqLimit, 
									 double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime )
{
	lowFrequency = lowFreqLimit;
	highFrequency = highFreqLimit;

	double pi = 3.14159265359;
	double twoPi = 2.0 * pi;
	double e = 2.718281828459045;
	CD ii( 0.0, 1.0 );

	// Auditory threshold at 1000 Hz re Praat is P0 = 1.0e-5 Pa, which Praat assumes by formula to be x=1, relative to full-scale 16-bits = 32768.
	double inputScale = 1.0 / 32768.0;
//	double P0 = 1.0e-5;

	// Return memory time = 1/ lowest bandwidth
	*pTheMemoryTime = 1.0 / ( twoPi * lowFrequency );

	// Calculate the frequencies
	Size = 0;
	double lastBandwidth = lowFrequency;
	double lastFrequency = lowFrequency;
	for ( double f = lowFrequency; f <= highFrequency; f += max( minSpacing, f * proportionalSpacing ) )
	{
		designFrequencies.push_back( f );
		
		// 1. Bandwidth should be at most f, so that negative pole is suppressed.
		// 2. Bandwidths should not change too fast, so that convergence works (estimated frequency determined on similar domain of time).
		// 3. There is a max bandwidth
		double bandwidth = f < maxBandwidth ? f : maxBandwidth;
		designBandwidths.push_back( bandwidth );
		lastBandwidth = bandwidth;
		lastFrequency = f;

		Size++;
	
		// The norm depends on tau
		double tau = 1.0 / (twoPi * bandwidth);


		// Series sum of the envelope is not the same as the full series-sum of the decaying sine wave, which should be about 0.
		// sum = (dt^3 exp(dt/tau) (1 + 4 exp(dt/tau) + exp((2 dt)/tau))) / (-1 + exp( dt/tau))^4
		double dttau = tau > 0.0 ? samplingInterval / tau : 0.0;
		double denom = pow( exp(dttau)-1.0, 4.0 );
		double A = pow( samplingInterval, 3.0 ) * exp( dttau );
		double B = 1.0 + 4.0 * exp( dttau ) + exp( 2.0 * dttau );
		double seriesSum = denom > 0.0 ? A * B / denom : 1.0;

		double norm = inputScale / (1.0 * seriesSum);

		// Calculate. See Mathematica derivation.
		CD k =-2.0 * ii * f * samplingInterval * pi - samplingInterval / tau;
		CD ca1 =-4.0 * pow(e, k);
		CD ca2 = 6.0 * pow( e, 2.0 * k);
		CD ca3 =-4.0 * pow( e, 3.0 * k);
		CD ca4 = pow( e, 4.0 * k);
		CD cb1 = pow(e, k) * norm;
		CD cb2 = 4.0 *  pow(e, 2.0 * k) * norm;
		CD cb3 = pow(e, 3.0 * k) * norm;
	
		// Add to coefficient vectors
		a1.push_back( ca1 );
		a2.push_back( ca2 );
		a3.push_back( ca3 );
		a4.push_back( ca4 );
		b1.push_back( cb1 );
		b2.push_back( cb2 );
		b3.push_back( cb3 );
	}
	
	// Build a vector of the filter poles
	filterPoles.clear();
	for ( size_t i = 0; i < designFrequencies.size(); i++ )
	{
	
		double r = exp( -pi * designBandwidths[i] * samplingInterval );
		double real = cos( samplingInterval * 2.0 * pi * designFrequencies[i] );
		double imag = -sin( samplingInterval * 2.0 * pi * designFrequencies[i] );
		CD pole ( r * real, r * imag );
		filterPoles.push_back( pole );
	}

	// allocate and zero out the memories
	x1 = CD(0.0,0.0);
	x2 = CD(0.0,0.0);
	x3 = CD(0.0,0.0);
	y1.assign( Size, CD(0.0,0.0) );
	y2.assign( Size, CD(0.0,0.0) );
	y3.assign( Size, CD(0.0,0.0) );
	y4.assign( Size, CD(0.0,0.0) );

	out.assign( Size, CD(0.0,0.0) );
	return Size;
}

Gammatone4PoleArray::~Gammatone4PoleArray()
{
	// not sure if this is needed, but it can't hurt.
	designFrequencies.clear();
	designBandwidths.clear();
	a1.clear();
	a2.clear();
	a3.clear();
	a4.clear();
	b1.clear();
	b2.clear();
	b3.clear();
	out.clear();
}

BOOL Gammatone4PoleArray::Process( double audioIn)
{
	// Real part is real input
	CD in(audioIn, 0.0 );
	for ( size_t i = 0; i < Size; i++ )
	{
		out[i] = -(a1[i] * y1[i] + a2[i] * y2[i] + a3[i] * y3[i] + a4[i] * y4[i]) + b1[i] * x1 + b2[i] * x2 + b3[i] * x3;
	}
	y4 = y3;
	y3 = y2;
	y2 = y1;
	y1 = out;

	x3 = x2;
	x2 = x1;
	x1 = in;
	return TRUE;
}
//----------------------------------------------------------------------------------------------------------------

ComplexGaussian::ComplexGaussian()
{
}

double ComplexGaussian::G( double x )
{
	double offset = ( FWN - 1.0 ) / 2.0;
	double t = ( x - offset ) * samplingInterval;

	return exp( -t * t/ ( 4.0 * sigma * sigma ) );
}

// Returns size of kernel. See wikipedia Window functions/ compact gaussian
// Very long kernels are possible, so supply a maximum size; probably no more than o(10^4).
long ComplexGaussian::Initialize( double aSamplingInterval, double frequency, double aSigma, long maxSize )
{
	if ( aSigma <= 0.0 )		return -1;
	if (aSamplingInterval <= 0.0 ) return -1;
	sigma = aSigma;
	samplingInterval = aSamplingInterval;

	double pi = 3.14159265359;
	double twoPi = 2.0 * pi;

	// Let's reference the midpoint for phase 0. 
	// FWN should be odd. We need at least FWN >= 7 * sigma.
	FWN = min( maxSize, (long) ( 7.0 * ( sigma / samplingInterval ) ) );
	// force to integer odd
	long FWN2 = (long) FWN/2;
	FWN = 2 * FWN2 + 1;
	size_t iFWN = (size_t) FWN;
	kernel.assign( iFWN, CD( 0.0, 0.0 ) );
	sines.assign( iFWN, 0.0 );
	cosines.assign( iFWN, 0.0 );
	envelope.assign( iFWN, 0.0 );
	
	// fill the kernel
	for ( size_t i = 0; i < iFWN; i++ )
	{
		double n = (double) i;
		double t = ( n - FWN2 ) * samplingInterval;
		double s = sin( twoPi * frequency * t );
		double c = cos( twoPi * frequency * t );
		double env = G(n) - ( G(0.5)*( G(n+FWN)+G(n-FWN) ) )/( G(-0.5+FWN) + G(-0.5-FWN) );
		sines[i] = env * s;
		cosines[i] = env * c;
		kernel[i] = CD( c * env, s * env );
		envelope[i] = env;
	}

	// allocate and zero out the memory
	inBuffer.assign( iFWN, 0.0 );
	cPointer = 0;
	return iFWN;
}

// convolve with circular buffer oldest to newest.
BOOL ComplexGaussian::Process( double audioIn )
{
	// update circular buffer
	circularUpdate( cPointer, inBuffer.size() );
	inBuffer[ cPointer ] = audioIn;
	cout = 0.0;
	for ( size_t i = 0; i < inBuffer.size(); i++ )
	{
		double x = inBuffer[ (size_t) circularIndex(i, cPointer, inBuffer.size()) ];
		cout = cout + x * kernel[ i ];
	}

	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------
ComplexGaussianArray::ComplexGaussianArray() : FilterBankArray()
{
}

// Returns count of filters
// The FWHM width (in wikipedie entry) looks like 1.43 * 2 sigma
const long MAXKERNELTOMAKE = 1000;
const double EMPIRICAL_WIDTH_COEFF = 1.43;
long ComplexGaussianArray::Initialize( double samplingInterval, double maxBandwidth, double lowFreqLimit, double highFreqLimit, 
									 double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime )
{
	lowFrequency = lowFreqLimit;
	highFrequency = highFreqLimit;

	// Auditory threshold at 1000 Hz re Praat is P0 = 1.0e-5 Pa, which Praat assumes by formula to be x=1, relative to full-scale 16-bits = 32768.
//	double inputScale = 1.0 / 32768.0;

	// Calculate the frequencies
	Size = 0;
	int maxKernelSize = 0;
	filterArray.clear();
	for ( double f = lowFrequency; f <= highFrequency; f += max( minSpacing, f * proportionalSpacing ) )
	{
		designFrequencies.push_back( f );
		
		// 1. Bandwidth should be at most f, so that negative pole is suppressed.
		// 2. Bandwidths should not change too fast, so that convergence works (estimated frequency determined on similar domain of time).
		// 3. There is a max bandwidth
		double bandwidth = f < maxBandwidth ? f : maxBandwidth;
		designBandwidths.push_back( bandwidth );
		Size++;
	
		// The the bandwidth is uses as straignt 1/time here -- no 2 pi needed.
		double tau = 1.0 / bandwidth;
		double sigma = ( tau / 2.0 ) / EMPIRICAL_WIDTH_COEFF;

		// Create the filters in the filterBank
		ComplexGaussian* pcg = new ComplexGaussian();
		long kernelSize = pcg->Initialize( samplingInterval, f, sigma, MAXKERNELTOMAKE );
		filterArray.push_back( pcg );
		maxKernelSize = max( maxKernelSize, kernelSize );
	}
	
	// Return memory time longest kernel
	*pTheMemoryTime = maxKernelSize;

	// There are no filter poles
	filterPoles.clear();
	out.assign( Size, CD(0.0,0.0) );
	return Size;
}

BOOL ComplexGaussianArray::Process( double audioIn)
{

	for ( size_t i = 0; i < Size; i++ )
	{
		filterArray[i]->Process( audioIn );
		out[i] = filterArray[i]->cout;
	}
	return TRUE;
}

ComplexGaussianArray::~ComplexGaussianArray()
{
	for ( size_t i = 0; i < filterArray.size(); i++ )
		delete filterArray[i];
}

//----------------------------------------------------------------------------------------------------------------

DifferentialConjugateProduct::DifferentialConjugateProduct()
{
}

BOOL DifferentialConjugateProduct::Initialize( size_t Size)
{
	x1.assign( Size, CD(0.0,0.0) );
	R00.assign( Size, 0.0 );
	R01.assign( Size, CD(0.0,0.0) );
	return TRUE;
}

BOOL DifferentialConjugateProduct::Process( CVECTOR *pin )
{
	for ( size_t i = 0; i < pin->size(); i++ )
	{
		R00[i] = norm( (*pin)[i] );
		R01[i] = x1[i] * conj((*pin)[i]);
	}
	x1 = *pin;
	return TRUE;
}

DifferentialConjugateProduct::~DifferentialConjugateProduct()
{
	x1.clear();
	R00.clear();
	R01.clear();
}

//----------------------------------------------------------------------------------------------------------------

FrequencyFlowInference::FrequencyFlowInference()
{
}

FrequencyFlowInference::~FrequencyFlowInference()
{
}

BOOL FrequencyFlowInference::Initialize( RS_DISPLAY_CHOICE aDisplayChoice, DVECTOR *pDesignBandwidthsIn, DVECTOR *pDesignCenterFrequencies, FrequencyScale *pScale, 
										size_t frequencyDisplayHeight, double aBandwidthScale, double aLocalityScale )
{
	displayChoice = aDisplayChoice;
	pCenters = pDesignCenterFrequencies;
	pDesignBandwidths = pDesignBandwidthsIn;
	ptheScale = pScale;
	frequencyDisplaySize = frequencyDisplayHeight;
	bandwidthDisplayScale = aBandwidthScale;
	localityScale = aLocalityScale;

	return TRUE;
}
	
// Need default: energy at full bandwidth. This shades into energy at indicated frequency, low-zero bandwidth.
BOOL FrequencyFlowInference::Process( DVECTOR *pEstimatedFrequencies, DVECTOR *pIntensity)
{
	displayWidth.clear();
	convergence.clear();
	localIntensity.clear();
	H0Entropy.clear();

	size_t N = pEstimatedFrequencies->size();
	double minF = (*pCenters)[0];
	double maxF = (*pCenters)[ pCenters->size()-1 ];

	for ( size_t i = 0; i < N; i++ )
	{
		// Compute convergence-- with end cases
		double c1 = i > 0   ? ( (*pEstimatedFrequencies)[i] - (*pEstimatedFrequencies)[i-1]) / ((*pCenters)[i]-(*pCenters)[i-1]) : 0.0;
		double c2 = i < N-1 ? ( (*pEstimatedFrequencies)[i+1] - (*pEstimatedFrequencies)[i]) / ((*pCenters)[i+1]-(*pCenters)[i]) : 0.0;
		double c = c1 + c2;
		if ( i > 0 && i < N-1 ) 
		{
			c = c/2.0;
		}
		convergence.push_back( c );
	}
	
	// Interpolate entropy per filter to display scale ( 0 to maxF), for the the H0 case.
	size_t j = 0;
	double lastCF = 0.0;
	double lastE = 0.0;
	double dF = ( maxF - minF ) / frequencyDisplaySize;
	for ( size_t i = 0; i < N; i++ )
	{
		double cf = (*pCenters)[i];
		double e = (*pIntensity)[i];
		double b = (*pDesignBandwidths)[i];
		e = e / b;
		for ( double f = lastCF; f < cf; f += dF )
		{
			if ( j++ >= frequencyDisplaySize ) break;		// guard
			double interp = lastE + ( f - lastCF ) / ( cf - lastCF ) * ( e - lastE );
			H0Entropy.push_back( interp );
		}
		lastCF = cf;
		lastE = e;
	}

	// Computed widths for display, computed localIntensity
	for ( size_t i = 0; i < N; i++ )
	{
		double center = (*pCenters)[i];
		double f = (*pEstimatedFrequencies)[i];

		// Map convergence to width as [0,1] --> [0,BW]
		double c = convergence[i];
		double width;
		double b = (*pDesignBandwidths)[i];

		if ( abs(c) <= 0.0001 )
		{
			// Narrow band signals have small values that may dip negative. We should declare minimum resolution at top level. 
			width = b * 0.0001;
		} else if ( c <= 0.0 )
		{
		} else
			
			// Convergence can be negative-- i.e. incorrect folding. We can't display that.
			width = b;
		{
			c = min( 1.0, c);
			width = c * b;
		}
		displayWidth.push_back( (float) width );
		
		// Apply a locality function to the intensity from this filter. Values far from center are not likely to be valid or are noisy.
		// This vector holds the intensity for this filter, computed for THERE, whereever that is.
		double nCycles = center/ b;
		double scale = localityScale / nCycles;

		double intensity = (*pIntensity)[i];
		localIntensity.push_back( fuzzyLocality( f, center, localityScale * b ) * intensity );
//	NEW??	localIntensity.push_back( fuzzyLocality( f, center, scale ) * intensity );
	}

	// Compute Resonance Spectrum: the local information sum of information intensity, localized by indicated frequency, and diffused by estimated bandwidth
	displaySpectrum.clear();
	displaySpectrum.assign( frequencyDisplaySize, 0.0 );


	// we could compute each display frequency and look at data to either side
	// or we could look at data and spread the values within this vector at frequencies. Need min/max frequency in either case.
	// data-driven gets us local designbandwidth for automatic local limits. Also allows zeros to sit there without computation.

	// Iterate over all the filters
	for ( size_t i = 0; i < N; i++ )
	{
		// Isolate the data we need. We will assume that all heuristic limits and so on have been applied
		double estimatedF = (*pEstimatedFrequencies)[i];
		
		if ( estimatedF < minF ) 
			continue;

		if ( displayChoice == RS_RESONANCE_SPECTRUM )
		{
			double intensity = localIntensity[i];
			double displayBW = displayWidth[i];
			if ( intensity > 0.0 && displayBW > 0.0 && estimatedF > minF )
				writeTriangleToDisplay( estimatedF, bandwidthDisplayScale * displayBW, intensity );
		} else
		{
			double intensity = (*pIntensity)[i];
			// No estimation of BW, no inference
			if ( intensity > 0.0 )
				writeTriangleToDisplay( estimatedF, bandwidthDisplayScale * (*pDesignBandwidths)[i], intensity );
		}
	}
	
	// H1 vs H0 is the difference.
	if ( displayChoice == RS_RESONANCE_SPECTRUM )
	{
		for ( size_t i = 0; i < displaySpectrum.size(); i++ )
		{
			float decisionVal = displaySpectrum[i] - (float) H0Entropy[i];
// test view			float decisionVal = (float) H0Entropy[i];
			displaySpectrum[i] = max( 0.0f, decisionVal );
		}
	}

	// Compute and save the min,max for later normalization
	spectrumMax = - DBL_MAX;
	spectrumMin = DBL_MAX;
	for ( int i = 0; i < (int) displaySpectrum.size(); i++ )
	{
		double d = displaySpectrum[i];
		spectrumMax = max( spectrumMax, d );

		// For mins, we ignore zeros
		if ( d > 0.0 )
			spectrumMin = min( spectrumMin, d );
	}
	// 
	return TRUE;
}

void FrequencyFlowInference::writeTriangleToDisplay( double estimatedF, double displayBW, double intensity )
{

	// Compute the frequency range to distribute. We will use a triangle with half-height at bandwdith. We make sure that it is centered at an integer
	double fLow = estimatedF - displayBW;
	double fHigh = estimatedF + displayBW;
	float localIntensity = (float) intensity / (float) displayBW;

	// Scale and Quantize
	int jLow = (int) ptheScale->Scale( fLow );
	int jHigh = (int) ptheScale->Scale( fHigh );
	int jCenter = (int) ptheScale->Scale( estimatedF );

	// minimum line size is 1!
	if( jHigh <= jLow+1 )
	{
		jLow = min( jLow, (int) frequencyDisplaySize-1 );
		displaySpectrum[jLow] += (float) localIntensity;
		return;
	}
	float triangleIncrement = jCenter > jLow ? localIntensity / ( jCenter - jLow ) : 0;
	float y = 0;
	for ( int j = jLow; j < jCenter; j++ )
	{
		displaySpectrum[j] += y;
		y += triangleIncrement;
	}
	
	displaySpectrum[ jCenter ] += localIntensity;

	// Now down
	triangleIncrement = jHigh > jCenter ? localIntensity / ( jHigh - jCenter ) : 0;
	y = localIntensity;

	for ( int j = jCenter+1; j <= jHigh; j++ )
	{
		displaySpectrum[j] += y;
		y -= triangleIncrement;
	}
	return;
}
//----------------------------------------------------------------------------------------------------------------

ScaleForDisplay::ScaleForDisplay()
{
}

ScaleForDisplay::~ScaleForDisplay()
{
}

BOOL ScaleForDisplay::Initialize( DVECTOR *pDesignCenterFrequencies, FrequencyScale *pScale, 
										size_t frequencyDisplayHeight, double lowFLimit, double highFLimit, BOOL aDoLimit )
{
	pCenterFrequencies = pDesignCenterFrequencies;
	ptheScale = pScale;
	frequencyDisplaySize = frequencyDisplayHeight;
	lowFrequencyLimit = lowFLimit;
	highFrequencyLimit = highFLimit;
	doLimit = aDoLimit;
	return TRUE;
}

BOOL ScaleForDisplay::Process( DVECTOR *pSpectrum )
{
	size_t N = pSpectrum->size();
	
	displaySpectrum.clear();
	displaySpectrum.assign( frequencyDisplaySize, 0.0 );
	
	double lastIntensity = 0.0;
	size_t j = 0;
	for ( size_t i = 0; i < N; i++ )
	{
		double estimatedF = (*pCenterFrequencies)[i];
		
		// Limit as required
		if ( doLimit )
		{
			if ( estimatedF < lowFrequencyLimit || estimatedF > highFrequencyLimit )
				continue;
		}
		double intensity = (*pSpectrum)[i];
		size_t jCenter = (size_t) ptheScale->Scale( estimatedF );
		double increment = jCenter > j ? ( intensity - lastIntensity ) / (jCenter - j) : 0.0;
		double value = lastIntensity;
		// Assuming scaled F goes forward, i.e. scale is monotonic, we interpolate the possible gap
		
		while( j < jCenter && j < frequencyDisplaySize )
		{
			displaySpectrum[j] = (float) ( value );
			value += increment;
			j++;
		}

		lastIntensity = intensity;
	}
	
	// Compute and save the min,max for later normalization
	spectrumMax = 0.0;
	spectrumMin = DBL_MAX;
	for ( int i = 0; i < (int) displaySpectrum.size(); i++ )
	{
		double d = displaySpectrum[i];
		spectrumMax = d != 0 ? max( spectrumMax, d ) : spectrumMax;
		spectrumMin = d != 0 ? min( spectrumMin, d ) : spectrumMin;
	}
	// 
	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------
FFTSpectrum::FFTSpectrum()
{
}

FFTSpectrum::~FFTSpectrum()
{
}

long FFTSpectrum::Initialize(int anOrder, double fractionalStep, double samplingInterval)
{
	const double PI = 3.1415926536;
	order = anOrder;
	fftSize = (int) pow( 2, order );
	long outputSize= fftSize/2 - 1;
	step = (int) ( fractionalStep * (double) fftSize );
//	delay = overlap /2;
	displayLowFrequency =  1.0 / ( samplingInterval * (double) fftSize/2 );
	displayHighFrequency = 1.0  / ( 2.0 * samplingInterval );

	// Blackman window
	window.clear();
	const double alpha = 0.16;
	double a0 = ( 1.0 - alpha ) / 2.0;
	double a1 = 0.5;
	double a2 = alpha / 2.0;
	double N = (double) fftSize;
	for ( size_t i = 0; i < fftSize; i++ )
	{
		double w = a0 - a1 * cos( ( 2.0 * PI * (double) i ) / ( N-1.0) ) + a2 * cos(4.0 * PI * (( double) i )/ (N-1.0));
		window.push_back( max(0.0, w) );
	}
	out.assign( fftSize/2-1, 0.0 );
	signalBuffer.assign( fftSize+1, 0.0 );
	FFT.assign( fftSize, CD(0.0,0.0) );
	cIndex = 0;
	cSize = signalBuffer.size()-1;
	counter = 0;

	// Compute these for documentation only
	designBandwidths.clear();
	designFrequencies.clear();
	filterPoles.clear();
	const CD J(0, 1);
	for ( size_t i = 1; i < fftSize/2; i++ )
	{
		double fRad = 2.0 * PI * i / fftSize;
		double f= fRad / ( 2.0 * PI * samplingInterval );
		designFrequencies.push_back( f );

		// This is pure estimate of the FWHM bandwidth of the window, from rough observation. It is constant.
		double b = 5.0 /( (double) fftSize/2.0 * samplingInterval );
		designBandwidths.push_back( b );

		// Poles are equivalent poles-- the window has many poles.
		filterPoles.push_back( exp(-b/ (2.0 * PI) ) * exp( -J * fRad ) );
	}

	return outputSize;
}

BOOL FFTSpectrum::Process( double audioIn )
{
	// Slide the data into the buffer
	int newest = circularIndex( cSize, cIndex, cSize);
	signalBuffer[ newest ] = audioIn;
	circularUpdate( cIndex, cSize );

	// Do we process?  Do so every "step" number of points
	if ( ++counter < step )
		return TRUE;
	else
		counter = 0;

	// Window and bitreverse and load the FFT buffer
	for ( size_t i = 0; i < fftSize; i++ )
	{
		int next = circularIndex( i, cIndex, cSize );
		double wx = window[i] * signalBuffer[ next ];
		FFT[bitReverse(i, order)] = wx;
	}
	
	// Do the butterflies
	fft( &FFT, order );

	for ( size_t i = 1; i < fftSize/2; i++ )
	{
		// Output as a filterbank for positive frequencies > 0
		out[i-1] = FFT[i];
	}
	return TRUE;
}

unsigned int FFTSpectrum::bitReverse(unsigned int x, int order)
{
	int n = 0;
//	int mask = 0x1;
	for (int i=0; i < order; i++)
	{
		n <<= 1;
		n |= (x & 1);
		x >>= 1;
	}
	return n;
}

// This is in-place, complex, radix-2, decimation in time
void FFTSpectrum::fft( CVECTOR *b, int log2n)
{
	const double PI = 3.1415926536;
	const CD J(0, 1);
	int n = 1 << log2n;
//	for (unsigned int i=0; i < n; ++i)
//	{
//		b[bitReverse(i, log2n)] = a[i];
//	}
	for (int s = 1; s <= log2n; ++s)
	{
		int m = 1 << s;
		int m2 = m >> 1;
		CD w(1, 0);
		CD wm = exp(-J * (PI / m2));
		for (int j=0; j < m2; ++j)
		{
			for (int k=j; k < n; k += m)
 			{
				CD t = w * (*b)[k + m2];
				CD u = (*b)[k];
				(*b)[k] = u + t;
				(*b)[k + m2] = u - t;
			}
			w *= wm;
		}
	}
}