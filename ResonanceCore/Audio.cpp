// Audio.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "Audio.h"
#include <propvarutil.h>

#include "Logger.h"
#include "MFapi.h"
#include "mmreg.h"

// Other non-MS codecs. This is another GUID for which I do not understand why I need to have my own.
//#define WAVE_FORMAT_FLAC    0xF1AC
//#define WAVE_FORMAT_FLAC   {0000F1AC-0000-0010-8000-00AA00389B71}
//DEFINE_MEDIATYPE_GUID(MFAudioFormat_FLAC, WAVE_FORMAT_FLAC);
GUID MyMFAudioFormat_FLAC = {0x0000F1AC,0x0000,0x0010,0x80,0x00,0x00,0xAA, 0x00,0x38,0x9B,0x71};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
// global for convenience of logging
extern Logger *theLogger;

// AudioFoundation


// Audio construction

AudioFoundation::AudioFoundation()
{
}

// Audio initialization

BOOL AudioFoundation::Initialize()
{
	// Initialize the Media Foundation platform
	HRESULT hr = MFStartup(MF_VERSION);
	if ( hr == MF_E_BAD_STARTUP_VERSION )
	{
		Report(CString( "Incompatible version of Media Foundation") );
		return FALSE;
	} else if ( hr != S_OK )
	{
		Report(CString( "Can't start up Media Foundation") );
		return FALSE;
	}
return TRUE;
}

BOOL AudioFoundation::Shutdown()
{
	MFShutdown();
	return TRUE;
}

void AudioFoundation::Report(CString s)
{
	CString announce;
	announce += "AudioFoundation:  ";
	announce += s;
	theLogger->output(announce);
}


// AudioSource
AudioSource::AudioSource()
{
	pReader = NULL;
	pMediaBuffer = NULL;
	pMediaSample = NULL;
	pPCMAudio = NULL;
	PCMSamplingRate = 0.0;
	channels = 0;   
	samplesPerSecond = 0;  
	currentBufferIndex = 0;
	currentBufferLength = 0;
	bitsPerSample = 0;
	bytesToRead =0;
	audioBufferChannel1.clear();
	audioBufferChannel2.clear();
	currentAbsolutePosition = 0;
	currentSamplePosition = 0;
	requestedStartPosition = 0;
	ticksPerSample = 0;
	fileDuration = 0.0;
}

AudioSource::~AudioSource()
{
	if ( pReader != NULL )
		ReleaseReader();		// just in case
}

BOOL AudioSource::Create(const wchar_t * lpszPathName)
{
	ReleaseReader();
	
	// MFCreateSourceReaderFromURL fails with access violation when file has 0 bytes. We need to do a test file open.
	HANDLE hFile = CreateFile( lpszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	LARGE_INTEGER fSize;
	if ( hFile == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		WCHAR buffer[128];
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,0, buffer,128,NULL);
		Report(buffer );
		CloseHandle( hFile );
		return FALSE;
	}
	GetFileSizeEx( hFile, &fSize );
	if ( fSize.QuadPart ==  0)
	{
		//CString s;	
		//s.Format( _T("Can't open media source from URL %s"), lpszPathName );	
		Report( _T("File size is zero: can't use.") );
		CloseHandle( hFile );
		return FALSE;
	}

	// Create the source reader to read the input file.
	HRESULT hr = MFCreateSourceReaderFromURL(lpszPathName, NULL, &pReader);
	if (FAILED(hr))
	{
		CString s;	
		s.Format( _T("Can't open media source from URL %s"), lpszPathName );	
		Report(s);
		return FALSE;
	}

	SetFileDuration();
	fileDuration = GetDurationSeconds();
	CString s;
	s.Format( _T("%s opened, duration is %10.5f seconds."), lpszPathName, fileDuration );
	Report(s);
	ReportAttributes();
	if (!ConfigureAudioStream() )
	{
		Report( L"Can't configure stream" );
		return FALSE;
	}
	// test here-- we should be PCM now.
	ReportAttributes();

	// Prime data pump!
//	InitializeForGetNext();
//	ProcessNextSample();
	return TRUE;
}

BOOL AudioSource::ReleaseReader()
{
	SafeRelease( &pMediaBuffer);
	SafeRelease( &pMediaSample);
	SafeRelease(&pReader);
	return TRUE;
}

void AudioSource::Report(CString s)
{
	CString announce;
	announce += "AudioSource:  ";
	announce += s;
	theLogger->output(announce);
}

double AudioSource::GetPCMSamplingRate()
{
	return PCMSamplingRate;
}

// ConfigureAudioStream
//
// From AudioClip sample code: Selects an audio stream from the source file, and configures the
// stream to deliver decoded PCM audio.

BOOL AudioSource::ConfigureAudioStream()
{
//    HRESULT hr = S_OK;

    IMFMediaType *pUncompressedAudioType = NULL;
    IMFMediaType *pPartialType = NULL;

    // Create a partial media type that specifies uncompressed PCM audio.
    if( FAILED( MFCreateMediaType(&pPartialType) ) )
		return FALSE;

     if( FAILED( pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio) ) )
		 return FALSE;

    if( FAILED( pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM) ))
		return FALSE;

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    if( FAILED( pReader->SetCurrentMediaType( (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPartialType ) ))
		return FALSE;

    // Get the complete uncompressed format.
    if( FAILED( pReader->GetCurrentMediaType( (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pUncompressedAudioType ) ))
		return FALSE;

    // Ensure the stream is selected.
    if( FAILED( pReader->SetStreamSelection( (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE ) ))
		return FALSE;

    // Save the PCM format
    pPCMAudio = pUncompressedAudioType;
    pPCMAudio->AddRef();
		
	// Examine as wave file and set key block size and bit parameters for multi-precision reads
	WAVEFORMATEX *pWF;
	UINT32 cbSize;
	if( FAILED( MFCreateWaveFormatExFromMFMediaType( pPCMAudio, &pWF, &cbSize ) ))
		return FALSE;
	bytesToRead = pWF->nBlockAlign;
	bitsPerSample = pWF->wBitsPerSample;

    SafeRelease(&pUncompressedAudioType);
    SafeRelease(&pPartialType);
    return TRUE;
}

// Returns success/fail, but also sets EOS
BOOL AudioSource::ReadSample( BOOL *pEndOfStream )
{
	DWORD streamIndex, flags;
	LONGLONG llTimeStamp;
	*pEndOfStream = FALSE;

	HRESULT hr = pReader->ReadSample(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,    // Stream index.
        0,                              // Flags.
        &streamIndex,                   // Receives the actual stream index. 
        &flags,                         // Receives status flags.
        &llTimeStamp,                   // Receives the time stamp.
        &pMediaSample                   // Receives the sample or NULL.
        );
	
    if (FAILED(hr))
    {
        // Assume end of file
        SafeRelease(&pMediaSample);
		*pEndOfStream = TRUE;
		return FALSE;
    }
	currentSamplePosition = llTimeStamp;
	currentAbsolutePosition = llTimeStamp;

//	CString s;
//    s.Format( L"Reading Stream %d (%I64d)...\n", (int) streamIndex, llTimeStamp);
//	Report(s);
    if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
    {
        Report(L"\tEnd of stream\n");
        SafeRelease(&pMediaSample);
		*pEndOfStream = TRUE;
		return TRUE;
    }
    if (flags & MF_SOURCE_READERF_NEWSTREAM)
    {
        Report(L"\tNew stream\n");
    }
    if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
    {
        Report(L"\tNative type changed\n");
    }
    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
    {
        Report(L"\tCurrent type changed\n");
    }
    if (flags & MF_SOURCE_READERF_STREAMTICK)
    {
        Report(L"\tStream tick\n");
    }

    if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
    {
		Report(L"\tFail: Format changed.");
     // The format changed. Reconfigure the decoder.
        //hr = ConfigureAudioStream(streamIndex);
       // if (FAILED(hr))
       // {
       //     return FALSE;
        //}
        SafeRelease(&pMediaSample);
		*pEndOfStream = TRUE;
		return FALSE;
	}

	return TRUE;
}

BOOL AudioSource::ProcessNextSample( BOOL *pEndOfStream )
{
	theLogger->debug( "ProcessNextSample");
	HRESULT hr = S_OK;

	// Read the next "Sample"
	if ( pReader == NULL )
	{
		theLogger->debug( "Reader disappeared!");
		return FALSE;
	}

    if (!ReadSample( pEndOfStream ) )
    {
        CString s;
		s.Format( L"ProcessSamples FAILED, hr = 0x%x\n", hr);
		Report(s);
		return FALSE;
	}
	if ( *pEndOfStream )
	{
		return TRUE;
	}
  	// Media Buffer data
	BYTE *pByteBuffer;
	DWORD cbMaxLength;
	DWORD cbCurrentLength;

  	// Need to convert possible multiple current buffers to one we can use, then Lock and get pointer.
	pMediaSample->ConvertToContiguousBuffer( &pMediaBuffer );
	hr = pMediaBuffer->Lock(&pByteBuffer, &cbMaxLength, &cbCurrentLength );

	audioBufferChannel1.clear();
	audioBufferChannel2.clear();

	// Copy data locally-- while buffer is valid! Can then unlock the buffer. We read only Channel 1 of N.
	currentBufferLength = cbCurrentLength / bytesToRead;
	if ( bitsPerSample == 8 )
	{
		for ( DWORD i = 0; i < cbCurrentLength; i += bytesToRead)
		{
			audioBufferChannel1.push_back( (double) pByteBuffer[i] );
			if ( channels > 1 )
				audioBufferChannel2.push_back( (double) pByteBuffer[i + 1] );
		}

	} else if ( bitsPerSample == 16 )
	{

		// Use as PCM 16bits
		for ( DWORD i = 0; i < cbCurrentLength; i += bytesToRead)
		{
			INT16 bit16 = (pByteBuffer[i+1] * 256) + pByteBuffer[i];
			audioBufferChannel1.push_back( (double) bit16 );
			if ( channels > 1 )
			{
				bit16 = (pByteBuffer[i+3] * 256) + pByteBuffer[i+2];
				audioBufferChannel2.push_back( (double) bit16 );
			}
		}
	} else if ( bitsPerSample == 24 )
	{
	for ( DWORD i = 0; i < cbCurrentLength; i += bytesToRead)
		{
			// All this is to extend the sign bit
			INT32 bit24 = Convert24Bit( pByteBuffer[i], pByteBuffer[i+1], pByteBuffer[i+2] );
			audioBufferChannel1.push_back( (double) bit24 );
			if ( channels > 1 )
			{
			bit24 = Convert24Bit( pByteBuffer[i+3], pByteBuffer[i+4], pByteBuffer[i+5] );
			audioBufferChannel2.push_back( (double) bit24 );
			}
		}
	} else
		return FALSE;

	currentBufferIndex = 0;
	pMediaBuffer->Unlock();
    return TRUE;
}

INT32 AudioSource::Convert24Bit( BYTE b0, BYTE b1, BYTE b2 )
{
	// All this is to extend the sign bit
	INT32 bit24 = 0;
	if ( b2 > 128 )
	{
		bit24 = 255 << 24;
		bit24 += b2 << 16;
		bit24 += b1 << 8;
		bit24 += b0;
	} else
	{
		bit24 += b2 << 16;
		bit24 += b1 << 8;
		bit24 += b0;
	}
	return bit24;
}

BOOL AudioSource::InitializeForGetNext()
{
	// Unlock and release the Media buffer
	if ( pMediaBuffer )
	{
		pMediaBuffer->Unlock();
	}
	SafeRelease( &pMediaBuffer );

	// Done with Sample for now.
    SafeRelease(&pMediaSample);
	return TRUE;
}

// This is not guaranteed to get to the exact position. Outputs flag for endOfStream.
BOOL AudioSource::Seek( double timeInSecs, BOOL *pEndOfStream )
{
	INT64 iTime = (INT64) (timeInSecs/ 1.0e-7);		// this is 8 bytes like VT_I8
	*pEndOfStream = FALSE;

	// Set precision time to start
	requestedStartPosition = iTime;
	currentAbsolutePosition = 0;
	currentSamplePosition = 0;

	// Unlock and release the Media buffer
	if ( pMediaBuffer )
	{
		pMediaBuffer->Unlock();
	}
	SafeRelease( &pMediaBuffer );

	// Done with Sample for now.
    SafeRelease(&pMediaSample);

	// Flush?
    if ( FAILED( pReader->Flush( MF_SOURCE_READER_FIRST_AUDIO_STREAM ) ) )
	{
		return FALSE;
	}

	// SetPosition-- this is only approximate, and we have no idea where it ends up yet! Default is 100 nsec units.
	PROPVARIANT varPosition;
	INT64 zTime = 0;
	InitPropVariantFromInt64(zTime, &varPosition );
	if ( FAILED( pReader->SetCurrentPosition( GUID_NULL, varPosition ) ))
		return FALSE;
	CString s;
	s.Format( L"Seek to  = %9.7f\n", timeInSecs);
	Report(s);

	currentBufferLength = 0;
	currentBufferIndex = 0;

	// Step forward to exact alignment
	while ( currentAbsolutePosition < requestedStartPosition-ticksPerSample )
	{
		double dummyVal1;
		double dummyVal2;
		if ( !GetNextPCMSample( &dummyVal1, &dummyVal2,  pEndOfStream ) )
		{
			return FALSE;
		}
		if ( *pEndOfStream )
			break;
	}
	return TRUE;
}

// We process up to two channels. Returns end of stream flag
BOOL AudioSource::GetNextPCMSample( double *pValueChan1, double *pValueChan2, BOOL *pEndOfStream )
{
	*pEndOfStream = FALSE;

	if ( currentBufferIndex >= currentBufferLength )
	{
		InitializeForGetNext();

		// On end of file or error, we will get FALSE
		if ( !ProcessNextSample( pEndOfStream ) )
		{
			return FALSE;
		}
		if ( *pEndOfStream )
		{
			return TRUE;
		}
	}
	// Stero wav has interleaved samples. At least get one channel!
	currentAbsolutePosition += ticksPerSample;		// might be better to be double

	if ( channels > 1 )
	{
		*pValueChan1 = audioBufferChannel1[ currentBufferIndex ];
		*pValueChan2 = audioBufferChannel2[ currentBufferIndex++ ];
	}
	else
	{
		*pValueChan1 = audioBufferChannel1[ currentBufferIndex++ ];
		*pValueChan2 = 0.0;
	}
	return TRUE;
}

BOOL AudioSource::SetFileDuration()
{
    PROPVARIANT var;
    HRESULT hr = pReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,  MF_PD_DURATION, &var);
    if (SUCCEEDED(hr))
    {
        hr = PropVariantToInt64(var, &duration);
        PropVariantClear(&var);
		return TRUE;
    }
    return FALSE;
}

BOOL AudioSource::ReportAttributes()
{
	CString reportString;
	HRESULT hr;

	PROPVARIANT var;
	WCHAR mimeType[128];
    if (SUCCEEDED(pReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,  MF_PD_MIME_TYPE, &var)))
    {
        hr = PropVariantToString(var, mimeType, 128);
        PropVariantClear(&var);

		CString s;
		s.Format( _T("Mime type is %s"), mimeType );
		Report(s);
    }

	UINT32 bitrate = 0;
    hr = pReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,  MF_PD_AUDIO_ENCODING_BITRATE, &var);
    if (SUCCEEDED(hr))
    {
        hr = PropVariantToUInt32(var, &bitrate);
        PropVariantClear(&var);

		CString s;
		s.Format( _T("Bit rate is %d"), (int) bitrate );
		Report(s);
    }

	IMFMediaType *pMediaType = NULL;
	GUID audioType;
	hr = pReader->GetCurrentMediaType( MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMediaType );
	if ( hr == S_OK )
	{
		pMediaType->GetItem( MF_MT_SUBTYPE, &var);
		CString sType;
		PropVariantToGUID( var, &audioType);
		if ( audioType == MFAudioFormat_Base ) sType = "Base";
		else if ( audioType == MFAudioFormat_PCM ) sType = "PCM";
		else if ( audioType ==	MFAudioFormat_Float ) sType = "Uncompressed IEEE floating-point audio";
		else if ( audioType ==	MFAudioFormat_DTS ) sType = "Digital Theater Systems (DTS) audio";
		else if ( audioType ==	MFAudioFormat_Dolby_AC3_SPDIF ) sType = "Dolby AC-3 audio over Sony/Philips Digital Interface (S/PDIF)";
		else if ( audioType ==	MFAudioFormat_DRM ) sType = "Encrypted audio data used with secure audio path";
		else if ( audioType ==	MFAudioFormat_WMAudioV8 ) sType = "Windows Media Audio 8, 9, or 9.1 codec";
		else if ( audioType ==	MFAudioFormat_WMAudioV9 ) sType = "Windows Media Audio 9 or 9.1 Professional codec";
		else if ( audioType ==	MFAudioFormat_WMAudio_Lossless ) sType = "Windows Media Audio 9 or 9.1 Lossless codec";
		else if ( audioType ==	MFAudioFormat_WMASPDIF ) sType = "Windows Media Audio 9 Professional codec over S/PDIF";
		else if ( audioType ==	MFAudioFormat_MSP1 ) sType = "Windows Media Audio 9 Voice codec";
		else if ( audioType ==	MFAudioFormat_MP3 ) sType = "MPEG Audio Layer-3 (MP3)";
		else if ( audioType ==	MFAudioFormat_MPEG ) sType = "MPEG-1 audio payload";
		else if ( audioType ==	MFAudioFormat_AAC ) sType = "Advanced Audio Coding (AAC)";
		else if ( audioType ==	MyMFAudioFormat_FLAC ) sType = "Media Foundation FLAC";
		else if ( audioType ==	MFAudioFormat_ADTS ) sType = "Not used";
		else sType = "Unknown";
		
		CString s;
		s.Format( _T("Audio format is %s.  "), sType );
		reportString = s;	
	}
		
	// Examine as wave file
	if ( audioType == MFAudioFormat_PCM )
	{
		WAVEFORMATEX *pWF;
		UINT32 cbSize,
		hr = MFCreateWaveFormatExFromMFMediaType( pMediaType, &pWF, &cbSize );
		
		CString s;
		s.Format( _T("Wav Formatted: Bits per sample = %5d Bytes per block %2d "), (int) pWF->wBitsPerSample, (int) pWF->nBlockAlign );
		reportString += s;

	}


//MF_MT_SAMPLE_SIZE	Size of each sample, in bytes.
	UINT32 Bytes;   
	UINT32 indep;
	UINT32 compressed;
	UINT32 fixedSize;
	CString s;


	if (SUCCEEDED( pMediaType->GetUINT32(  MF_MT_SAMPLE_SIZE, &Bytes)))
   {
		s.Format( _T("Bytes per sample = %5d "), Bytes );
		reportString += s;
    }

  
    if (SUCCEEDED( pMediaType->GetUINT32(  MF_MT_AUDIO_NUM_CHANNELS, &channels)))
	{
		s.Format( _T("Number of channels = %2d "), (int) channels );
		reportString += s;
    }

    if (SUCCEEDED( pMediaType->GetUINT32(  MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond)))
   {
		s.Format( _T("Samples per second = %6d "), (int) samplesPerSecond );
		reportString += s;
		PCMSamplingRate = (double) samplesPerSecond;
		ticksPerSample = (long) ( 1.0e7 / PCMSamplingRate );
    }

	// MF_MT_COMPRESSED	Specifies whether the media data is compressed
    if (SUCCEEDED( pMediaType->GetUINT32( MF_MT_COMPRESSED, &compressed)))
    {
		s.Format( _T("Compressed = %1d "), (int) compressed );
		reportString += s;
	}

	// MF_MT_FIXED_SIZE_SAMPLES	Specifies whether the samples have a fixed size.
	hr = pMediaType->GetUINT32( MF_MT_FIXED_SIZE_SAMPLES, &fixedSize);
	if (SUCCEEDED( hr))
    {
		s.Format( _T("Samples fixed size = %1d "), (int) fixedSize );
		reportString += s;
	}
	
	// MF_MT_ALL_SAMPLES_INDEPENDENT	Specifies whether each sample is independent of the other samples in the stream.
    if (SUCCEEDED( pMediaType->GetUINT32( MF_MT_ALL_SAMPLES_INDEPENDENT, &indep)))
    {
		s.Format( _T("Samples independent = %1d "), (int) indep );
		reportString += s;
	}
	Report( reportString );

//Attribute	Description
//MF_MT_AM_FORMAT_TYPE	Format GUID.
//MF_MT_USER_DATA	Contains user data for a media type that was converted from a legacy format structure.
//MF_MT_WRAPPED_TYPE	Contains a media type that has been wrapped in another media type.

	return TRUE;
}