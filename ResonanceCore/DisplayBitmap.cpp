
// DisplayBitmap.cpp : implementation of the CDisplayBitmap class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ResonanceCore.h"
#endif

#include <Windows.h>

#include "Coordinator.h"
#include "DisplayBitmap.h"
#include "resource.h"

#include <math.h>
//#include "MainFrm.h"
#include <comdef.h>

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

DisplayBitmap::DisplayBitmap( Coordinator* parent )
{
	pTheCoordinator = parent;
	ptheParameterPack = pTheCoordinator->getParameterPack();

	brushSpectrumPalette.clear();
	pFactory = NULL;
	pWICBitmap = NULL;
	pWICFactory = NULL;
	pBitmapRenderTarget = NULL;
	pBitmapEncoder = nullptr;
	brushHeight = 0.0f;
	brushWidth = 0.0f;
}

BOOL DisplayBitmap::Release()
{
	brushSpectrumPalette.clear();
	SafeRelease( &pBitmapRenderTarget );
	SafeRelease( &pBitmapEncoder );
	SafeRelease( &pWICBitmap );
	SafeRelease( &pWICFactory );
	SafeRelease( &pFactory );
	return TRUE;
}

DisplayBitmap::~DisplayBitmap()
{
	Release();
	delete ptheDrawTimeFrequencyGrid;
	for ( size_t i = 1; i < brushSpectrumPalette.size(); i++ )
		delete brushSpectrumPalette[i];
}

BOOL DisplayBitmap::Initialize( UINT aBitmapHeight, UINT aBitmapWidth )
{
	Release();

	// Get display times and other information
	bitmapHeight = aBitmapHeight;
	bitmapWidth = aBitmapWidth;
	double alphaScale =	1.0;

	// Added code for factory here--odd place to put it.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if ( FAILED(hr)) return FALSE;
	
	// Create a DirectWrite factory.
//	hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(pDWriteFactory),
//            reinterpret_cast<IUnknown **>(&pDWriteFactory) );
//	 if ( FAILED(hr)) return FALSE;

	// Create a Windows Imaging Component WICBitmap. Note the "1" needed for Windows 7.
	 hr = CoCreateInstance( CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, 	IID_IWICImagingFactory,
		(LPVOID*) &pWICFactory );
	 if ( FAILED(hr)) return FALSE;
	

	// Could be CacheOnDemand-- i.e. allocate right here? 
	// GUID_WICPixelFormat32bppPBGRA is essential for D2D1 to work
	hr = pWICFactory->CreateBitmap( bitmapWidth, bitmapHeight, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad,
        &pWICBitmap );
	 if ( FAILED(hr)) return FALSE;

	// Create a WIC bitmap render target
	 D2D1_RENDER_TARGET_PROPERTIES bitmapProperties = {
		D2D1_RENDER_TARGET_TYPE_SOFTWARE,
		// fails D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM),		// sets unknown as defaults. not sure about this one
		D2D1::PixelFormat(),
		0.0f, 0.0f,													// default dpi
//		D2D1_RENDER_TARGET_USAGE_NONE,								// I have no idea what I am doing here
		D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
		D2D1_FEATURE_LEVEL_DEFAULT
	 };
     hr = pFactory->CreateWicBitmapRenderTarget( pWICBitmap, bitmapProperties, &pBitmapRenderTarget);
	if ( FAILED(hr) )
	{
		_com_error err( hr & 0x0000FFFF );
		LPCTSTR errMsg = err.ErrorMessage();	
		return FALSE;
	}
	SetSpectrumPalette( alphaScale, 10 );

	ptheDrawTimeFrequencyGrid = new DrawTimeFrequencyGrid();
	if ( !ptheDrawTimeFrequencyGrid->Initialize( 12.0 ) )
		return FALSE;	
	
	// Load the title image
	//hr = LoadResourceBitmap( pBitmapRenderTarget, pWICFactory, IDB_PNG1, L"PNG", &pTitleBitmap );
	return TRUE;
}

IWICBitmap* DisplayBitmap::RenderSpectrumBitmap( double displayLowRange, double displayHighRange, double peakEnhanceValue,
												double atime0, double atime1 )
{
	time0 = atime0;
	time1 = atime1;

	pBitmapRenderTarget->BeginDraw();
	pBitmapRenderTarget->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
	UINT bitmapWidth = 0;
	UINT bitmapHeight = 0;
	pWICBitmap->GetSize( &bitmapWidth, &bitmapHeight );

	// TEMPORARY __ CLEAN UP !!
	double deltaT  = (time1-time0) / (double) bitmapWidth;
	
//	displayDynamicRange = displayDynamicRange <= 0.0 ? 1.0 : displayDynamicRange;	// guard against user values

	// Core loops: frequency and time, then load a pixel. Frequency columns could be pre-computed once, by the same parameter rules as the signalprocessing.
	// Loop on: actual bitmap coordinates.
	// This is mainly a 1-1 select and color transform.

	// Find the screen max for normalization. Later, we need to do the file max too.
	// We assume positive values, with zero values indicated no-data.
	double screenMax = pTheCoordinator->getCanvasMax();
	double screenMin = pTheCoordinator->getCanvasMin();

	double scaledRange = 1;	// some value
	double scaleLow = 0.0;
	double scaleHigh = scaledRange;
	scaleBreak = screenMax * 0.0000001;
	if ( screenMax > 0.0 && screenMax < DBL_MAX )
	{
		scaleHigh = scaler( screenMax );
	}
	if ( screenMin > 0.0 )
	{
		scaleLow =	 scaler(screenMin);
	}
	scaledRange = abs( scaleHigh - scaleLow );
	

	// NEW Flood with zero color by FillRectangle
	// Then paint non-zero points -- iterating over DATA so as to never miss data.
	pBitmapRenderTarget->FillRectangle( D2D1::RectF((float) bitmapWidth, (float) bitmapHeight ), brushSpectrumPalette[0] );

	// set brushsize to cover
	FVECTOR * spectrum =   pTheCoordinator->getSpectrumAtTime( time0 );
	if ( spectrum == nullptr )
	{
		pBitmapRenderTarget->EndDraw();
		return pWICBitmap;
	}
	size_t spectrumSize = spectrum->size();

	// blocky one: float defaultBrushSize = max( 1.0, (double) bitmapHeight / ( double ) spectrumSize );
	brushHeight = 1.0f;
	brushWidth = (float) max( 1.0, (time1-time0) / (deltaT * (double) bitmapWidth ));

	// Loop over time, render each spectrum vector
	double t = time0;
	workingBuffer.assign( spectrumSize, 0.0f );
	workingBuffer2.assign( spectrumSize, 0.0f );
	for ( double g = 0.0; g < (double) bitmapWidth; g += 1 ) 
	{
		// Interpolate from last vertical pixel (bottom)
		float lastY = (float) bitmapHeight;
		float lastX = 0.0;

		FVECTOR * spectrum =   pTheCoordinator->getSpectrumAtTime( t );
		t += deltaT;
		if ( spectrum == nullptr ) break;

		// Mung the numbers
		for ( size_t i = 0; i < spectrumSize; i++ )
		{
			double x = (double) (*spectrum)[i];

			// scale the intensity values
			if ( x > 0.0 )
				x = ( -scaleLow + scaler( x ) ) / ( scaledRange );

			// now limit/stretch as directed by user values
			x = max( x - displayLowRange, 0.0 );
			x = min( displayHighRange, x );
			x = x / ( displayHighRange - displayLowRange );
			workingBuffer[i] = (float) x;
		}
		ContrastEnhance( peakEnhanceValue, &workingBuffer, &workingBuffer2 );

		// Paint the vertical
		for ( int i = 0; i < (int) spectrumSize; i++ )
		{
			double x = workingBuffer2[i];
			double fraction = ((double) i + 1.0 )/ (double) spectrumSize;

			// Get bitmap height address, backwards (screen addresses go top to down!)
			fraction = 1.0 - fraction;
			UINT iY = (UINT) ( fraction * (double) bitmapHeight );
			float thisY = min ((float) iY, (float) bitmapHeight - 1.0f );
			thisY = max( thisY, 0.0f );

			if ( lastY == bitmapHeight || lastY - 1.0f <= thisY )
			{
				// Only one to do
				Paint( (float) x, (float) g, thisY );
			}
			else
			{
				// Paint from just after last location to (including) this one, interpolating intensity
				for ( float fy = lastY - 1.0f; fy >= thisY; fy -= 1.0f )
				{
					float proportion = ( lastY - fy ) / ( lastY - thisY );
					float interpolatedX = proportion * (float) x + (1.0f - proportion) * ( lastX );
					Paint( interpolatedX, (float) g, fy );
				}
			}
			lastY = thisY;
			lastX = (float) x;
		}
	}
	//PasteLogo();

	pBitmapRenderTarget->EndDraw();
	return pWICBitmap;
}

void DisplayBitmap::Paint( float intensity, float fx, float fy )
{
	// Map to our color spectrum of brushes.
	int brushColorIndex = (int) ( intensity  * ( double ) brushSpectrumPalette.size() );
	brushColorIndex = min( brushColorIndex, (int) brushSpectrumPalette.size()-1 );
	brushColorIndex = max( brushColorIndex, 0 );

	//D2D1_ELLIPSE ellipse = D2D1::Ellipse( D2D1::Point2F( (float) g, fy), brushSize, brushSize );
	// left, top, right bottom
	D2D1_RECT_F rectangle = D2D1::RectF( fx, fy+brushHeight, fx + brushWidth, fy );
	pBitmapRenderTarget->FillRectangle(rectangle, brushSpectrumPalette[brushColorIndex]);
}

// Build the spectrum palette for levels to colors. We use the standard "jet" palette
void DisplayBitmap::SetSpectrumPalette( double alpha, int interpolationLevels)
{
	brushSpectrumPalette.clear();

// Definition of the colors.	
//	#00007F: dark blue
//	#0000FF: blue
//	#007FFF: azure
//	#00FFFF: cyan
//	#7FFF7F: light green
//	#FFFF00: yellow
//	#FF7F00: orange
//	#FF0000: red
//	#7F0000: dark red
	const float redMap[9]		= {0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f };
	const float greenMap[9]		= {0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f };
	const float blueMap[9]		= {0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f };

	ID2D1SolidColorBrush    *pBrush;

	for ( size_t i = 1; i < 9; i++ )
	{
		for ( int j = 0; j < interpolationLevels; j++ )
		{
		float fraction = ((float) j) / (float) (interpolationLevels);
		float redVal =		(1.0f-fraction) * redMap[i-1]   + fraction * redMap[i];
		float greenVal =		(1.0f-fraction) * greenMap[i-1] + fraction * greenMap[i];
		float blueVal =		(1.0f-fraction) * blueMap[i-1] + fraction * blueMap[i];

		pBitmapRenderTarget->CreateSolidColorBrush(D2D1::ColorF(redVal, greenVal, blueVal, (float) alpha), &pBrush);
		brushSpectrumPalette.push_back( pBrush );
		}
	}
	// Add the last one (full dark red)
	pBitmapRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.5f, 0.0f, 0.0f, (float) alpha), &pBrush);
	brushSpectrumPalette.push_back( pBrush );
}

BOOL DisplayBitmap::onSaveDisplayAs( CString outputFilePath)
{
	// Draw the grid before we put out
	pBitmapRenderTarget->BeginDraw();
	D2D1_SIZE_F rectangleSize = pBitmapRenderTarget->GetSize();
	D2D1_RECT_F rc = D2D1::RectF(0.0, 0.0, rectangleSize.width, rectangleSize.height );
	ptheDrawTimeFrequencyGrid->Draw( pBitmapRenderTarget, rc, ptheParameterPack, time0, time1 );
	pBitmapRenderTarget->EndDraw();

	
	// For jpeg output
	HRESULT hr = pWICFactory->CreateEncoder( GUID_ContainerFormatJpeg, nullptr, &pBitmapEncoder );
	if ( FAILED(hr)) return FALSE;

	IWICStream *pStream;

	pWICFactory->CreateStream(&pStream);
	
	// Initialize the stream using the output file path
	pStream->InitializeFromFilename( outputFilePath, GENERIC_WRITE );
	
	// Create encoder to write to image file
	pBitmapEncoder->Initialize(pStream, WICBitmapEncoderNoCache);

	//Get and create image frame
	IWICBitmapFrameEncode *pEncodeFrame;

	pBitmapEncoder->CreateNewFrame(&pEncodeFrame, nullptr);

	// Set options here. NONE OF THIS BEATS THE DEFAULT FILESIZE. Photoshop can cut quite a bit more.
/*	IPropertyBag2 *pIEncoderOptions;
	pBitmapEncoder->CreateNewFrame(&pEncodeFrame, &pIEncoderOptions);

	PROPBAG2 option = { 0 };
    option.pstrName = L"JpegYCrCbSubsampling";
    VARIANT varValue;    
    VariantInit(&varValue);
    varValue.vt = VT_UI1;
    varValue.bVal = WICJpegYCrCbSubsamplingDefault;      
    hr = pIEncoderOptions->Write(1, &option, &varValue);        

	pEncodeFrame->Initialize(pIEncoderOptions);
*/
	//Initialize the encoder from the bitmap. 
	pEncodeFrame->Initialize(nullptr);

	UINT bitmapWidth = 0;
	UINT bitmapHeight = 0;
	pWICBitmap->GetSize( &bitmapWidth, &bitmapHeight );
	pEncodeFrame->SetSize(bitmapWidth, bitmapHeight);
	WICPixelFormatGUID pixelFormat;
	pWICBitmap->GetPixelFormat( &pixelFormat );
	pEncodeFrame->SetPixelFormat(&pixelFormat);

	// Write bitmap to output
	pEncodeFrame->WriteSource(pWICBitmap, nullptr);
    pEncodeFrame->Commit();
	pBitmapEncoder->Commit();

	pEncodeFrame->Release();
	pBitmapEncoder->Release();
	pStream->Release();
	/*
	if (FAILED(hr))
	{
		TRACE( " Couldn't Save File: %s, %x " , (LPCTSTR) strPath, hr);
		return  FALSE;
	} */
   return  TRUE;
}

void DisplayBitmap::ContrastEnhance( double peakEnhanceValue, FVECTOR *pSpectrum, FVECTOR* pOut )
{
	double floor = 0.0;

	// Display running average to work on contrast enhancement	
	for ( int i = 0; i < (int) pSpectrum->size(); i++ )
	{
		int minj = max( 0, i-30 );
		int maxj = min( (int) pSpectrum->size() - 1, minj + 60 );
		double sum = 0.0;
		for ( int j = minj; j <= maxj; j++ )
		{
			sum += (*pSpectrum)[j];
		}
		double a = sum / ( 1 + maxj - minj );

		double s = (*pSpectrum)[i] -  peakEnhanceValue * a;
		(*pOut)[i] = (float) max( floor, s );
	}

}

void DisplayBitmap::PasteLogo()
{
	D2D1_SIZE_F fsize = pTitleBitmap->GetSize();

	float aspectRatio = fsize.height / fsize.width;
	float targetFraction = 0.15f;

	UINT bitmapWidth = 0;
	UINT bitmapHeight = 0;
	pWICBitmap->GetSize( &bitmapWidth, &bitmapHeight );

	float destWidth = targetFraction * (float) bitmapWidth;
	
	// Need a minimum resolution for appearance
	destWidth = max( 200, destWidth );

	float destHeight = destWidth * aspectRatio;
	
	// Copy all of icon to right lower corner
	D2D1_RECT_F sourceRectangle = D2D1::RectF(0.0f, 0.0, fsize.width, fsize.height);
	D2D1_RECT_F destinationRectangle =  	D2D1::RectF( 
			(float) bitmapWidth - destWidth,
			(float) bitmapHeight - destHeight, 
			(float) bitmapWidth, 
			(float) bitmapHeight);

	pBitmapRenderTarget->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
	pBitmapRenderTarget->DrawBitmap( pTitleBitmap, destinationRectangle, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, sourceRectangle);
}

HRESULT DisplayBitmap::LoadResourceBitmap( ID2D1RenderTarget *pRenderTarget, IWICImagingFactory *pIWICFactory,
   int resourceID, LPCTSTR resourceType, ID2D1Bitmap **ppBitmap )
{
    IWICBitmapDecoder *pDecoder = NULL;
    IWICBitmapFrameDecode *pSource = NULL;
    IWICStream *pStream = NULL;
    IWICFormatConverter *pConverter = NULL;
    IWICBitmapScaler *pScaler = NULL;

    HRSRC imageResHandle = NULL;
    HGLOBAL imageResDataHandle = NULL;
    void *pImageFile = NULL;
    DWORD imageFileSize = 0;

    // Locate the resource.
	imageResHandle = FindResourceW(NULL, (LPCTSTR) resourceID, resourceType );
    HRESULT hr = imageResHandle ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        // Load the resource.
        imageResDataHandle = LoadResource(GetModuleHandle(NULL), imageResHandle);
        hr = imageResDataHandle ? S_OK : E_FAIL;
    }
	if (SUCCEEDED(hr))
    {
        // Lock it to get a system memory pointer.
        pImageFile = LockResource(imageResDataHandle);
        hr = pImageFile ? S_OK : E_FAIL;
    }
	if (SUCCEEDED(hr))
    {
        // Calculate the size.
        imageFileSize = SizeofResource(GetModuleHandle(NULL), imageResHandle);
        hr = imageFileSize ? S_OK : E_FAIL;
    }
	
	// create stream
	if (SUCCEEDED(hr))
    {
          // Create a WIC stream to map onto the memory.
        hr = pIWICFactory->CreateStream(&pStream);
    }
    if (SUCCEEDED(hr))
    {
        // Initialize the stream with the memory pointer and size.
        hr = pStream->InitializeFromMemory( reinterpret_cast<BYTE*>(pImageFile), imageFileSize );
    }

	if (SUCCEEDED(hr))
   {
       // Create a decoder for the stream.
        hr = pIWICFactory->CreateDecoderFromStream( pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDecoder );
    }
	if (SUCCEEDED(hr))
   {
        // Create the initial frame.
        hr = pDecoder->GetFrame(0, &pSource);
    }
	if (SUCCEEDED(hr))
    {
        // Convert the image format to 32bppPBGRA
        // (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }
	if (SUCCEEDED(hr))
    {
		 hr = pConverter->Initialize( pSource, GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut );
	}
	if (SUCCEEDED(hr))
    {
        //create a Direct2D bitmap from the WIC bitmap.
        hr = pRenderTarget->CreateBitmapFromWicBitmap( pConverter, NULL, ppBitmap );
    }

    SafeRelease(&pDecoder);
    SafeRelease(&pSource);
    SafeRelease(&pStream);
    SafeRelease(&pConverter);
    SafeRelease(&pScaler);

    return hr;
}