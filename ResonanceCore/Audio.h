
// Audio.h : interface of the Audio class
//

#pragma once

#include "ResonanceCore.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>
#include <vector>
//#include <string>
#include <atlstr.h>
using namespace std ;

// Encapsulates Media Foundation behavior
class AudioFoundation
{
public:
	AudioFoundation();
	~AudioFoundation();

// Attributes
protected:
public:

// Operations
public:
	BOOL Initialize();
	BOOL Shutdown();

// Overrides
	public:

// Implementation
public:


private:
	void Report(CString s);

};

// Holds and encapsulates one instance of a MediaSourceReader
class AudioSource
{
public:
	AudioSource();
	~AudioSource();

// Attributes
protected:
public:

// Operations
public:

// Implementation
public:
		BOOL Create(const wchar_t * lpszPathName);
		BOOL ReleaseReader();
		BOOL ConfigureAudioStream();
		double GetDurationSeconds() { return ((double) duration ) * 1.0e-7; };
		BOOL GetNextPCMSample( double *pValueChan1, double *pValueChan2, BOOL *pEndOfStream );
		double GetPCMSamplingRate();		// samples per second. This is a not variable over time for PCM
		BOOL Seek( double timeInSecs, BOOL *pEndOfStream );

protected:
		IMFSourceReader *pReader;
		IMFMediaType *pPCMAudio;   // Receives the audio format.

private:
		LONGLONG duration;					// File duration in 100 ns units
		LONGLONG currentSamplePosition;		// This is the only solid index we get into the audio file. Comes from ReadSample.
		LONGLONG currentAbsolutePosition;			// Absolute individual audio sample index
		LONGLONG requestedStartPosition;		// Set on Seek(). 
		double fileDuration;

		double PCMSamplingRate;		// after conversion to PCM, this is the same as samplesPerSecond
		UINT32 channels;   
		UINT32 samplesPerSecond;  
		UINT32 ticksPerSample;		// in 100 ns ticks

private:
	BOOL SetFileDuration();
	BOOL ReadSample( BOOL *pEndOfStream );
	BOOL ProcessNextSample( BOOL *pEndOfStream );
	BOOL ReportAttributes();
	void Report( CString s );
	BOOL InitializeForGetNext();
	INT32 Convert24Bit( BYTE b0, BYTE b1, BYTE b2 );

	// Interfaces
    IMFSample *pMediaSample;
	IMFMediaBuffer *pMediaBuffer;

	// Locally buffered PCM 16-bit signed ints
	DWORD currentBufferLength;
	DWORD currentBufferIndex;
	vector <double> audioBufferChannel1;
	vector <double> audioBufferChannel2;

	int bytesToRead;
	int bitsPerSample;

};