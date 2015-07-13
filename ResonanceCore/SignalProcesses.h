// SignalProcesses.h : Interface for the signal processing objects

// For simplicity, we can put the allocation/dealloc and the parameter passing in the constructor/destructor.
// Initialize will zero memories and so on.
// Shutdown may be useful...TBD

// If we end up with a lot of these, we can go to more general containers.
//

#pragma once
#include "ResonanceCore.h"
#include "ParameterPack.h"

//helper functions/macros
#define quo(x, y) ( ( (y) != 0.0 ) ? ( (x) / (y) ) : 0.0 )
#define circularUpdate(cposition, clength)		cposition = (cposition+1)%(clength)
#define circularIndex(position, cposition, clength)	((position + cposition )%(clength))

// abstract class
class FilterBankArray
{
public:
	FilterBankArray();
	~FilterBankArray();
	
	virtual long Initialize( double samplingInterval, double maxBandwidth,  double lowFreqLimit, double highFreqLimit,
									double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime ) = 0;
	virtual BOOL Process( double audioIn ) = 0;

protected:
	size_t Size;

public:
	// This information is very useful in other processes and places
	double lowFrequency;
	double highFrequency;
	DVECTOR designBandwidths;
	DVECTOR designFrequencies;
	CVECTOR filterPoles;		// for snap only
	
	// Complex filter outputs
	CVECTOR out;
};

class Gammatone1PoleArray : public FilterBankArray
{
public:
	Gammatone1PoleArray();
	~Gammatone1PoleArray();
	
	long Initialize( double samplingInterval, double maxBandwidth,  double lowFreqLimit, double highFreqLimit,
									double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime );
	BOOL Process( double audioIn );

private:
	CVECTOR a1;

	// Memory vectors. Note that y is vector, but x is scalar
	CVECTOR y1;
};

class Gammatone4PoleArray : public FilterBankArray
{
public:
	Gammatone4PoleArray();
	~Gammatone4PoleArray();
	
	long Initialize(double samplingInterval, double maxBandwidth, double lowFreqLimit, double highFreqLimit, 
									 double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime );
	BOOL Process( double audioIn );

private:
	CVECTOR a1;
	CVECTOR a2;
	CVECTOR a3;
	CVECTOR a4;
	CVECTOR b1;
	CVECTOR b2;
	CVECTOR b3;

	// Memory vectors. Note that y is vector, but x is scalar
	CVECTOR y1;
	CVECTOR y2;
	CVECTOR y3;
	CVECTOR y4;
	CD x1;
	CD x2;
	CD x3;
};

// This one we'd better build out of an array of objects
class ComplexGaussian
{
public:
	ComplexGaussian();
//	~ComplexGaussian();
	long Initialize( double samplingInterval, double frequency, double sigma, long maxSize );
	BOOL Process( double audioIn );

private:
	double G( double x );

private:
	double FWN;
	double sigma;
	double samplingInterval;
	CVECTOR kernel;

	DVECTOR envelope;		// for testing and doc
	DVECTOR sines;
	DVECTOR cosines;

	// Memory buffer -- can be real-- is circular. size must be same as kernel size
	DVECTOR inBuffer;
	int cPointer;

public:
	CD cout;
};
typedef vector<ComplexGaussian*> GAUSSIAN_ARRAY;

//FIR confined-Gaussian window quadrature filter
class ComplexGaussianArray : public FilterBankArray
{
public:
	ComplexGaussianArray();
	~ComplexGaussianArray();
	
	long Initialize(double samplingInterval, double maxBandwidth, double lowFreqLimit, double highFreqLimit, 
									 double proportionalSpacing,	 double minSpacing, double *pTheMemoryTime );
	BOOL Process( double audioIn );

private:
	GAUSSIAN_ARRAY filterArray;
};

//----------------------------------------------------------------------------------------------------------------

class DifferentialConjugateProduct
{
public:
	DifferentialConjugateProduct();
	~DifferentialConjugateProduct();
	BOOL Initialize(size_t size);
	BOOL Process( CVECTOR *pin );

private:
	CVECTOR x1;

public:
	DVECTOR R00;
	CVECTOR R01;
};
//----------------------------------------------------------------------------------------------------------------

// Examines (instantaneous) local frequency field and infers centers of excitation and their bandwidth
class FrequencyFlowInference
{
public:
	FrequencyFlowInference();
	~FrequencyFlowInference();
	BOOL Initialize(RS_DISPLAY_CHOICE displayChoice, DVECTOR *pDesignBandwidths, DVECTOR *pCenterFrequencies, FrequencyScale *pScale, 
		size_t frequencyDisplayHeight, double bandwidthScale, double localityScale );
	BOOL Process( DVECTOR *estimatedFrequencies, DVECTOR *pIntensity );

private:
	RS_DISPLAY_CHOICE displayChoice;
	DVECTOR *pDesignBandwidths;
	DVECTOR *pCenters;
	double bandwidthDisplayScale;
	size_t frequencyDisplaySize;
	double localityScale;
	FrequencyScale *ptheScale;
	
	void writeTriangleToDisplay( double center, double bandwidth, double intensity );

public:
	DVECTOR H0Entropy;
	DVECTOR displayWidth;			// BW Full width to display, in Hz, by channel
	FVECTOR displaySpectrum;			// Summed entropy at parametrically given frequency resolution
	DVECTOR convergence;
	DVECTOR localIntensity;
	double spectrumMax;				// max of the above vector
	double spectrumMin;

};

//----------------------------------------------------------------------------------------------------------------
// Applies current system scale function, applies limits
class ScaleForDisplay
{
public:
	ScaleForDisplay();
	~ScaleForDisplay();
	BOOL Initialize( DVECTOR *pCenterFrequencies, FrequencyScale *pScale, 
		size_t frequencyDisplayHeight,  double lowFrequency, double highFrequency, BOOL doLimit );
	BOOL Process( DVECTOR *spectrum );

private:
	DVECTOR *pCenterFrequencies;
	FrequencyScale *ptheScale;
	size_t frequencyDisplaySize;
	double lowFrequencyLimit;
	double highFrequencyLimit;
	BOOL doLimit;

public:
	FVECTOR displaySpectrum;			// Result
	double spectrumMax;				// max of the above vector
	double spectrumMin;
};

//----------------------------------------------------------------------------------------------------------------
class FFTSpectrum
{
public:
	FFTSpectrum();
	~FFTSpectrum();
	long Initialize(int order, double fractionalStep, double samplingInterval);
	BOOL Process( double audioIn );
	CVECTOR* GetPoles() { return &FFT; };

private:
	unsigned int FFTSpectrum::bitReverse(unsigned int x, int order);
	void fft( CVECTOR *b, int log2n);

private:
	int order;
	size_t fftSize;
	int step;
//	int delay;
	int counter;
	int cIndex;
	int cSize;
	DVECTOR window;
	DVECTOR signalBuffer;
	CVECTOR FFT;

public:

	DVECTOR designBandwidths;
	DVECTOR designFrequencies;
	CVECTOR filterPoles;		


	double displayLowFrequency;
	double displayHighFrequency;

	// Standard complex output
	CVECTOR out;
};