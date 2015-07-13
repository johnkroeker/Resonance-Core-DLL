// Integrators.cpp : Definitions for the signal processing objects
//

#include "stdafx.h"
#include "ResonanceCore.h"
//#include "Integrators.h"
#include "Decoders.h"
#include <math.h>

//----------------------------------------------------------------------------------------------------------------
BOOL DecodeToFrequency::Initialize()
{
	return TRUE;
}

DecodeToFrequency::~DecodeToFrequency()
{
}

DecodeToFrequency::DecodeToFrequency( DVECTOR *pDesignCenterFrequencies, size_t Size, double aNoiseFloor, double samplingInterval )
{
	pCenterFrequencies = pDesignCenterFrequencies;
	noiseFloor = aNoiseFloor;
	deltaT = samplingInterval;

	// compute power level from parameter. Parameter is relative power for normalized input. Note that we convert from db = 10 log10.
	powerThreshold = pow( 10, 0.1 * noiseFloor );
	frequency0.assign( Size, 0.0 );
	frequency1.assign( Size, 0.0 );
	frequency2.assign( Size, 0.0 );
	poles0.assign( Size, 0.0 );
	poles1.clear();
	poles2.clear();
	timeShift.assign( Size, 0.0);
}
// Re-code!
inline double AngleInHertz( CD x, double dT ) { return atan2(x.imag(), x.real())/(6.283185307179586476925286766559*dT); };

void DecodeToFrequency::ProcessEntropy( DVECTOR *pR00 )
{
	entropyIntensity.clear();
	double totalAmplitude = 0.0;
	for ( size_t i = 0; i < pR00->size(); i++ )
	{
		double power = (*pR00)[i];
		if ( power > 0.0 )
		{
			double amplitude = sqrt(power);
			totalAmplitude += amplitude;
		}
	}
	
	// Compute LOCAL entropy. Keep power threshold here for now. Not consistent with totalPower!!
	// We have overlap to the extant of bandwidth/ delta centers. This
//	double overlap = 20.0;
	for ( size_t i = 0; i < pR00->size(); i++ )
	{
//		if ( i > 0 )
//		{
//			overlap = (*pDesignBandwidths)[i] / ( (*pCenterFrequencies)[i] - (*pCenterFrequencies)[i-1] );
//		}

		double entropy = 0.0;
		double power = (*pR00)[i];
		if ( totalAmplitude > 0.0 && power > 0.0 && power > powerThreshold )
		{
			double amplitude =  sqrt(power);
			double fractionalAmplitude = amplitude / totalAmplitude;
			
			// TEST ONLY
			//fractionalAmplitude /= overlap;
			double log2P = fractionalAmplitude > 0.0 ? 3.3219280948873623479 * log10( fractionalAmplitude ) : 0.0;
			entropy = - fractionalAmplitude * log2P;
		}
		entropyIntensity.push_back( (float) entropy );
	}
}

// Convention will be that an output frequency value of zero is a no-detect. Why filter here, better to do it all in "inference"
BOOL DecodeToFrequency::Process( DVECTOR *pR00, CVECTOR *pR10  )
{
	ProcessEntropy( pR00 );
	size_t N = pR10->size();

	poles0.assign( N, 0.0 );
	frequency0.assign( N, 0.0 );

	// Find the estimated frequencies at each filter.
	for ( size_t i = 0; i < N; i++ )
	{
		double power = (*pR00)[i];
		if ( power > 0.0 )
		{
			// Normalize the DCP sequential correlation.
			CD normalizedDCP = (*pR10)[i] / power;
			poles0[i] = normalizedDCP;
			frequency0[i] = abs(AngleInHertz( normalizedDCP, deltaT ));
		}
	}

	// Find the estimated timeshift at each filter. We ignore bandwidths for now.
	timeShift[0] = 0.0;
	timeShift[ N- 1 ] = 0.0;
	for ( size_t i = 1; i < N-1; i++ )
	{
		double fm1 = frequency0[ i-1 ];
		double f0 = frequency0[ i ];
		double fp1 = frequency0[ i+1 ];
		if ( f0 > 0.0 && fm1 > 0.0 )
		{
			double dCenter = (*pCenterFrequencies)[i] - (*pCenterFrequencies)[i-1];
			timeShift[i] = 1000.0 * quo( deltaT * ( f0 - fm1 ) , dCenter ); // for display in msec
		}
	}

	return TRUE;
}

// 3x3 case-- NOT READY YET. Looks like we need to assume something to get to the poles (a pole from from previous solms?)
BOOL DecodeToFrequency::Process(DVECTOR *pR00, CVECTOR *pR10, CVECTOR *pR20, CVECTOR *pR21, CVECTOR *pR30, CVECTOR *pR31, CVECTOR *pR32 )
{
	double pi = 3.14159265358979;
	ProcessEntropy( pR00 );

	poles0.assign( pR00->size(), 0.0 );
	poles1.assign( pR00->size(), 0.0 );
	poles2.assign( pR00->size(), 0.0 );
	frequency0.assign( pR00->size(), 0.0 );
	frequency1.assign( pR00->size(), 0.0 );
	frequency2.assign( pR00->size(), 0.0 );
	
	// Assume that R00 is good enough.
	DVECTOR *pR11 = pR00;
	DVECTOR *pR22 = pR00;
	DVECTOR *pR33 = pR00;

	// Find the estimated frequencies at each filter.
	for ( size_t i = 0; i < pR00->size(); i++ )
	{
		double p00 = (*pR00)[i];

		CD p10 = (*pR10)[i];
		double p11 = (*pR11)[i];

		CD p20 = (*pR20)[i];
		CD p21 = (*pR21)[i];
		double p22 = (*pR22)[i];

		CD p30 = (*pR30)[i];
		CD p31 = (*pR31)[i];
		CD p32 = (*pR32)[i];
		double p33 = (*pR33)[i];

		// Compute determinant. We divide this up due to suspicions of complex complex arithmatic
		double det1 = p11 * ( p22 * p33 - norm(p32) );
		CD triple = conj( p31 ) * p21 * p32;
		double det2 = norm(p21) * p33 + 2.0 *  triple.real() - p22 * norm( p31 );
		double det = det1 + det2;
		if ( det <= 0.0 )		// negative too??
		{
			continue;
		}

		// Compute alpha1
		CD alpha1 =		(( p22 * p33 - norm(p32) ) * p10) + 
						((conj(p31) * p32 + conj(p21) * p33) * p20 ) +
						((conj(p21) * conj(p32) - conj(p31) * p22) * p30 );
		alpha1 = alpha1 / det;

		// Compute alpha2
		CD alpha2 =	(conj(p32) * p31 - p21 * p33 ) * p10 +
					(p11 * p33 - norm(p31)) * p20 +
					(conj(p31) * p21 - p11 * conj(p32)) * p30;
		alpha2 = alpha2 / det;

		// Compute alpha3
		CD alpha3 =	(p21 * p32 - p22 * p31 ) * p10 +
					(conj(p21) * p31 - p11 * p32 ) * p20 +
					(p11 * p22 - norm(p21)) * p30;
		alpha3 = alpha3 / det;


		// REAL HYPPOTHESIS: Now find incoming pole
		double inReal = ( alpha1.real() - 0.0 ) / 2.0;
		double inImagUnder = -alpha2.real() - 2.0 * 0.0 * inReal - inReal * inReal;
		double inImag = inImagUnder > 0.0 ? sqrt( inImagUnder ) : sqrt( -inImagUnder );
		CD inPole( inReal, inImag );

		// The following assumes a two pole system
		// Build the poles, find frequencies.    **make sure we have positive frequencies
//		CD inner = alpha2+(alpha1*alpha1)/4.0;
//		CD radical = sqrt( inner );
//		CD pole1 = alpha1 /2.0 + radical;
//		CD pole2 = alpha1 /2.0 - radical;

		//double fEst = acos( realpart / 2 ) /( 2.0 * pi * deltaT );
		CD alpha1OnePole = p10 / p00;
		double fEst1 = abs(AngleInHertz( alpha1OnePole, deltaT ));

		double fEst = abs(AngleInHertz( alpha1, deltaT ));

		frequency0[i] = fEst1;
	}

	return TRUE;
}

// The 2x2 case
BOOL DecodeToFrequency::Process(DVECTOR *pR00, CVECTOR *pR10, CVECTOR *pR20, CVECTOR *pR21 )
{
	
	// Assume that R00 is good enough for all self-terms.
	DVECTOR *pR11 = pR00;
	DVECTOR *pR22 = pR00;

	ProcessEntropy( pR00 );
	
	poles0.assign( pR00->size(), 0.0 );
	poles1.assign( pR00->size(), 0.0 );
	poles2.assign( pR00->size(), 0.0 );
	frequency0.assign( pR00->size(), 0.0 );
	frequency1.assign( pR00->size(), 0.0 );
	frequency2.assign( pR00->size(), 0.0 );

	// Find the estimated frequencies at each filter.
	for ( size_t i = 0; i < pR00->size(); i++ )
	{
		double power = (*pR00)[i];
		double amplitude = power > 0.0 ? sqrt(power) : 0.0;

		// The two pole solution.
		TwoPolesSolutionStationary2( (*pR11)[i], (*pR22)[i], (*pR10)[i], (*pR20)[i], (*pR21)[i] );
		double f = abs(AngleInHertz( pole1, deltaT ));
		frequency0[i] = max( 0.0, f );

		if ( pole1.imag() < 0.0 )
		{
			pole1 = conj(pole2);
		}
		if ( pole2.imag() < 0.0 )
		{
			pole2 = conj(pole2);
		}
		poles0[i] = pole1;					// For snapshot
		poles1[i] = pole2;					// For snapshot
		f = abs(AngleInHertz( pole2, deltaT ));
		frequency1[i] = max( 0.0, f);
	}
	return TRUE;
}

double DecodeToFrequency::poleBandwidth( CD pole )
{
	double pi = 3.14159265358979;
	// abs(complex) is absurdly slow!
	double length = sqrt(norm(pole));
	if ( length > 1.0 ) length = 2 - length;
	if (length <= 0 ) return 0.0;
	return -log(length) / ( pi * deltaT );
}

//double EstimateInputSignal::residual()
//{
//	CD crossTerm = r21 * conj( alpha1 ) * alpha2;
//	double ss = r00 - (r11 * norm(alpha1) + r22 * norm(alpha2) + 2.0 * crossTerm.real() );
//	double r = quo( ss + MINIMUM_POWER, r00 );
//	if ( r <= 0.0 ) return 0.0;
//	return sqrt(r);
//}

// posts results in object locals
void DecodeToFrequency::TwoPolesSolutionStationary2( double r11, double r22, CD r10, CD r20, CD r21 )
{
	// Computations -- from research/xpd/projects/highres2006/SpeechReceiver.cpp
	CD r12 = conj( r21 );

	double determinant = r11 * r22 - norm(r21);
	if ( determinant <= 0.0 )
	{
		pole1 = 0.0;
		pole2 = 0.0;
		return;
	}
	double inverseD = determinant > 1.e-100 ? 1.0 / determinant : 0.0;

	CD t1 = r22 * r10 - r12 * r20;
	CD t2 = r11 * r20 - r21 * r10;

	// Predictor coefficients (for difference equation). This is perfectly general as modelling a difference equation, so far
	CD alpha1 = inverseD * conj( t1 );
	CD alpha2 = inverseD * conj( t2 );

	// Compute residual
//	CD crossTerm = r21 * conj( alpha1 ) * alpha2;
//	double ss = r00 - (r11 * norm(alpha1) + r22 * norm(alpha2) + 2.0 * crossTerm.real() );
//	double r = r00 > 0.0 ? (ss + noiseFloor)/ r00 : 0.0;
//	double residual = r > 0.0 ? sqrt(r) : 0.0;

	// Predict
//	CD predict = alpha1 * inputLag1 + alpha2 * inputLag2;

	// The following assumes a REAL?? two pole system
	// Build the poles, find frequencies.    **make sure we have positive frequencies
	CD inner = alpha2+(alpha1*alpha1)/4.0;
	CD radical = sqrt( inner );

	// Post results to parent
	pole1 = alpha1 /2.0 + radical;
	//pole2 = alpha1 /2.0 - radical;
	//pole2 = pole1 - alpha1;
	pole2 = alpha2 / pole1;

}

double DecodeToFrequency::residualForOnePole(double r00, double r11, CD alpha0 )
{
	double ss = r00 - ( r11 * norm(alpha0) );
	double r = r00 > 0.0 ? (ss + noiseFloor) / r00 : 0.0;
	if ( r < 0.0 ) return 1.0;
	return sqrt(r);
}
