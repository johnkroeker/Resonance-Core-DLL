
// ResonanceStudioDraw.cpp : implementation of the CResonanceStudioDraw class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ResonanceStudio.h"
#endif

#include "ResonanceStudioDoc.h"
#include "ResonanceStudioDraw.h"
#include <math.h>
#include "MainFrm.h"
//#include "ParameterPack.h"
//#include "DrawTimeFrequencyGrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

extern CMainFrame *pTheMainFrame;


// CResonanceStudioDraw

// CResonanceStudioDraw construction/destruction

ResonanceStudioDraw::ResonanceStudioDraw()
{
	pFactory = nullptr;
	pRenderTarget = nullptr;

	pAudioBrush = nullptr;
	pWICBitmap = nullptr;
	pbitmap = nullptr;
	ptheParameterPack = nullptr;
	ptheFrequencyScale = nullptr;
	ptheDrawTimeFrequencyGrid = nullptr;
//	
	lowFrequencyLimit = 0.0;
	highFrequencyLimit = 0.0;
	time0 = 0.0;
	time1 = 0.0;
	deltaT = 0.0;
	deltaPixels = 0.0;
	audioScreenFraction = 0.2;
	dataBottom = 0.0;
	dataTop = 0.0;

	extern 	CMainFrame *pTheMainFrame;
	pTheMainStatusBar = &(pTheMainFrame->m_wndStatusBar);
	panViewCorner = 0;
}

ResonanceStudioDraw::~ResonanceStudioDraw()
{
}

BOOL ResonanceStudioDraw::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// Added code for factory here--odd place to put it.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if ( FAILED(hr)) return FALSE;

	ptheDrawTimeFrequencyGrid = new DrawTimeFrequencyGrid();

	if ( !ptheDrawTimeFrequencyGrid->Initialize( 10.0 ) )
		return FALSE;

	return TRUE;
}

// CResonanceStudioDraw drawing
// Note the top left is origen
void ResonanceStudioDraw::Draw(HWND m_hWnd, RECT rc, CResonanceStudioDoc *pDoc )
{
	CWaitCursor wait;
	clientRectangle = rc;

	pTheCoordinator = pDoc->GetCoordinator();
	if ( pTheCoordinator == nullptr )
	{
		return;
	}
	ptheParameterPack = pTheCoordinator->GetParameterPack();
	ptheFrequencyScale = ptheParameterPack->GetFrequencyScale();
	
	CPropertiesWnd *pthePropertiesWnd = &(pTheMainFrame->m_wndProperties);
	pthePropertiesWnd->SetOutputStatus();

 	SetPalette();

	HRESULT hr = CreateGraphicsResources(m_hWnd, rc);

    if (SUCCEEDED(hr))
    {
    
        pRenderTarget->BeginDraw();

        // set the background and force a paint while we do our procesing
		pRenderTarget->Clear(backgroundColor);
		
		if ( pTheCoordinator->ProcessAndGetWICBitmap( &pWICBitmap ) )
		{
			// nothing to do. need actual screen size in renderTarget to map
		}
 
		// Get display times and other information
		highFrequencyLimit = ptheParameterPack->GetFilterHigh();
		lowFrequencyLimit = ptheParameterPack->GetFilterLow();
		rectangleSize = pRenderTarget->GetSize();
		dataBottom = rectangleSize.height;
		dataTop = audioScreenFraction * rectangleSize.height;

		if (pWICBitmap != nullptr )
		{
			// Copy WIC bitmap to ID2D1 bitmap
			hr = pRenderTarget->CreateBitmapFromWicBitmap( pWICBitmap, NULL, &pbitmap );

			time0 = pTheCoordinator->GetCurrentDisplayStartTime();
			time1 = pTheCoordinator->GetCurrentDisplayEndTime();
			double rate = pTheCoordinator->GetSamplingRate();

			SetStatus( time0, time1 );
			SetPixelTimeGrid( time1-time0, rate );

			// Use top 20% of screen for audio
			//const double xAxis = 0.1 * rectangleSize.height;
			double vScale = 0.2 * rectangleSize.height ;
		
			DrawAudioLine( 0.0, vScale );
 
			// Display the bitmap. Collect rectangle data first. Display to bottom section, below audio
			UINT uiWidth = 0;
			UINT uiHeight = 0;
			pWICBitmap->GetSize( &uiWidth, &uiHeight );
			
			D2D1_RECT_F sourceRectangle;
			CPropertiesWnd *pthePropertiesWnd = &(pTheMainFrame->m_wndProperties);
			if ( pthePropertiesWnd->GetFullScreenOrPan() == "Pan" )
			{
				// Limit travel
				float x = (float) panViewCorner.x;
				float y = (float) panViewCorner.y;
				x = max( x, 0.0f );
				y = max( y, 0.0f );
				x = min( x, uiWidth - rectangleSize.width );
				y = min( y, uiHeight - rectangleSize.height );
				sourceRectangle = D2D1::RectF(x, y,  x + rectangleSize.width, y + rectangleSize.height);
			}
			else
			{
				sourceRectangle = D2D1::RectF(0.0f, 0.0, (float) uiWidth, (float) uiHeight);
			}

			D2D1_RECT_F destinationRectangle = D2D1::RectF( 0.0f, (float) vScale, rectangleSize.width, rectangleSize.height);
			pRenderTarget->DrawBitmap(pbitmap, destinationRectangle,1.0F,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, sourceRectangle);

			ptheDrawTimeFrequencyGrid->Draw( pRenderTarget, D2D1::RectF(0.0, (float) dataTop, rectangleSize.height, (float) dataBottom ),
				ptheParameterPack, time0, time1 );
		}


        hr = pRenderTarget->EndDraw();
		wait.Restore();
        DiscardGraphicsResources();
	}
}

void ResonanceStudioDraw::DrawAudioLine( double yOrigen, double vScale )
{
	float lastx = 0;
	float lasty = 0;
	double lastT = time0;
	double signalWidth = 0.0;
	double signalEndVal = 0.0;

	double minRange = 0.0;
	double maxRange = 0.0;
	if ( !pTheCoordinator->GetSignalRange( time0, time1, &minRange, &maxRange ) )
		return;

	pTheCoordinator->GetSignalStroke( lastT, lastT, minRange, maxRange, &signalWidth, &signalEndVal );

	lasty = (float) (yOrigen + vScale - vScale * signalEndVal);
	
	// Must loop with precision, then go discrete
	for ( double g = 0; g < (double) rectangleSize.width; g += deltaPixels ) 
	{
		double newT = lastT + deltaT;

		pTheCoordinator->GetSignalStroke( lastT, newT, minRange, maxRange, &signalWidth, &signalEndVal );
		
		// Note that screen down is increasing in pixels
		float fy = (float) (yOrigen + vScale - vScale * signalEndVal);
		float width = (float)  max( 1.0, vScale * signalWidth );
		float fx = (float) g;
		if ( width <= 1.0 )
		{
			pRenderTarget->DrawLine( D2D1::Point2F(lastx,lasty), D2D1::Point2F(fx ,fy), pAudioBrush, 1.0f );
		} else
		{
			// Use Horizontal wide stroke
			float meanfy = (lasty+fy)/2.0f;
			pRenderTarget->DrawLine( D2D1::Point2F(lastx,meanfy), D2D1::Point2F(fx ,meanfy), pAudioBrush, width );
			pRenderTarget->DrawLine( D2D1::Point2F(lastx,lasty), D2D1::Point2F(fx ,fy), pAudioBrush, 1.0f );
		}
		lastx = fx;
		lasty = fy;
		lastT = newT;
		}
}

void ResonanceStudioDraw::SetPalette()
{
//	CPropertiesWnd *pthePropertiesWnd = &(pTheMainFrame->m_wndProperties);
//	CString palette = pthePropertiesWnd->GetPalette();
//	double alpha = 1.0;

//	if ( palette == "Oscilloscope" )
//	{
//		backgroundColor =		D2D1::ColorF(D2D1::ColorF::Black);
//		audioColor =				D2D1::ColorF(0.5f, 1.0f, 0.0f);
//		dividerColor =			D2D1::ColorF(1.0f, 0.0f, 0.0f);
//	} else
	{
		backgroundColor =		D2D1::ColorF(D2D1::ColorF::White);
		audioColor =				D2D1::ColorF(D2D1::ColorF::ForestGreen);
//		dividerColor =			D2D1::ColorF(1.0f, 0.0f, 0.0f);
	}
}

HRESULT ResonanceStudioDraw::CreateGraphicsResources(HWND m_hWnd, RECT rc)
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
	
		// Retrieve the size of the client area and create an ID2D1HwndRenderTarget of the same size that renders to the window's HWND. 
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hWnd, size), &pRenderTarget);
		if ( FAILED(hr) ) return hr;

		//pRenderTarget->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
        hr = pRenderTarget->CreateSolidColorBrush(audioColor, &pAudioBrush);            
	}
    return hr;
}

// If pixels > samples then draw lines between samples.
// If samples > pixels, then summarize with wide brush pixel by pixel.
void ResonanceStudioDraw::SetPixelTimeGrid( double requestedDisplayTimeInterval, double samplingRate )
{
	D2D1_SIZE_F size = pRenderTarget->GetSize();
//	double rate = pTheCoordinator->GetSamplingRate();

	double samples = samplingRate > 0.0 ? requestedDisplayTimeInterval * samplingRate : 0.0;
	if ( samples > size.width )
	{
		deltaT  = requestedDisplayTimeInterval / size.width;
		deltaPixels = 1.0;
	} else
	{
		deltaT = 1.0 / samplingRate;
		deltaPixels = size.width / samples;
	}
}

void ResonanceStudioDraw::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
	SafeRelease(&pAudioBrush);
//	SafeRelease(&pDividerBrush);
}
 
void ResonanceStudioDraw::SetStatus( double start, double end )
{
	//COLORREF is 0x00bbggrr
	CString s;
	s.Format(_T("Start %6.3f"), start );
	pTheMainStatusBar->SetPaneText( 3, s );
	pTheMainStatusBar->SetPaneTextColor(1, 0x00ee);
	pTheMainStatusBar->SetPaneBackgroundColor(1, 0x00eeeeee );
//	pTheMainStatusBar->SetPaneWidth( 1, 2 * pTheMainStatusBar->GetPaneWidth(1) );
	
	s.Format(_T("End %6.3f"), end );
	pTheMainStatusBar->SetPaneText( 4, s );
	pTheMainStatusBar->SetPaneTextColor(2, 0x00ee);
	pTheMainStatusBar->SetPaneBackgroundColor(2, 0x00eeeeee );
//	pTheMainStatusBar->SetPaneWidth(21, 2 * pTheMainStatusBar->GetPaneWidth() );

}

// gets the mouse  coordinates on right click. TODO: make this work on log-scale too!
void ResonanceStudioDraw::SetTimeFrequencyIndicators( CPoint point)
{
	// Mouse coordinates are in Client rectangle coords-- not D2D1 ones.
	double yFraction = ( (double) point.y - audioScreenFraction * clientRectangle.bottom ) / ( (1.0 - audioScreenFraction)  * clientRectangle.bottom);
	double frequency = highFrequencyLimit- (highFrequencyLimit - lowFrequencyLimit) * yFraction;

	double xFraction = (double) point.x / (double) clientRectangle.right;
	double time = time0 + (time1 - time0) * xFraction;

	//COLORREF is 0x00bbggrr
	CString s;
	s.Format(_T("Time %6.5f"), time );
	pTheMainStatusBar->SetPaneText( 1, s );
	pTheMainStatusBar->SetPaneTextColor(1, 0x00ee);
	pTheMainStatusBar->SetPaneBackgroundColor(1, 0x00eeeeee );
//	pTheMainStatusBar->SetPaneWidth( 1, 2 * pTheMainStatusBar->GetPaneWidth(1) );
	
	s.Format(_T("Frequency %6.3f"), frequency );
	pTheMainStatusBar->SetPaneText( 2, s );
	pTheMainStatusBar->SetPaneTextColor(2, 0x00ee);
	pTheMainStatusBar->SetPaneBackgroundColor(2, 0x00eeeeee );
//	pTheMainStatusBar->SetPaneWidth(21, 2 * pTheMainStatusBar->GetPaneWidth() );

}

// We pass through here to get screen coordinates translated to data units
void ResonanceStudioDraw::ShowSlice( CPoint point, SliceMode mode)
{
	// Mouse coordinates are in Client rectangle coords-- not D2D1 ones.
	double yFraction = ( (double) point.y - audioScreenFraction * clientRectangle.bottom ) / ( (1.0 - audioScreenFraction)  * clientRectangle.bottom);
	double frequency = highFrequencyLimit- (highFrequencyLimit - lowFrequencyLimit) * yFraction;

	double xFraction = (double) point.x / (double) clientRectangle.right;
	double time = time0 + (time1 - time0) * xFraction;
	pTheCoordinator->ShowSlice( frequency, time, mode );
}
