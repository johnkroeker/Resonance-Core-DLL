
// DrawTimeFrequencyGrid.cpp : implementation of the CDrawTimeFrequencyGrid class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ResonanceCore.h"
#endif

//#include "ResonanceStudioDoc.h"
#include "DrawTimeFrequencyGrid.h"
#include <math.h>
//#include "MainFrm.h"
#include "ParameterPack.h"

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

// DrawTimeFrequencyGrid

// DrawTimeFrequencyGrid construction/destruction

DrawTimeFrequencyGrid::DrawTimeFrequencyGrid()
{
	pFactory = nullptr;
	pRenderTarget = nullptr;

	pVGridBrush = nullptr;
	ptheFrequencyScale = nullptr;
	
	dataBottom = 0.0;
	dataTop = 0.0;
}

DrawTimeFrequencyGrid::~DrawTimeFrequencyGrid()
{
	SafeRelease( &pTextFormat );
	SafeRelease( &pDWriteFactory );
	SafeRelease( &pFactory );
}

BOOL DrawTimeFrequencyGrid::Initialize( float aFontSize )
{
	static const WCHAR msc_fontName[] = L"Verdana";
	fontSize = aFontSize;

	// Added code for factory here--odd place to put it.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if ( FAILED(hr)) return FALSE;
	
	// Create a DirectWrite factory.
	hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(pDWriteFactory),
            reinterpret_cast<IUnknown **>(&pDWriteFactory) );
	 if ( FAILED(hr)) return FALSE;

	 // Create a DirectWrite text format object.
     hr = pDWriteFactory->CreateTextFormat( msc_fontName, NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"", //locale
            &pTextFormat );
	 if ( FAILED(hr)) return FALSE;

	// Center the text horizontally and vertically.
	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	return TRUE;
}

// DrawTimeFrequencyGrid drawing grids. We leave the renderTarget context up to the caller. 
// Note the top left is origen
void DrawTimeFrequencyGrid::Draw( ID2D1RenderTarget *apRenderTarget, D2D1_RECT_F rc, ParameterPack *ptheParameterPack, 
					double time0, double time1 )
{
	pRenderTarget = apRenderTarget;

	ptheFrequencyScale = ptheParameterPack->GetFrequencyScale();
//	CString palette = ptheParameterPack->GetPalette();
	SetPalette();

    HRESULT hr = S_OK;
	hr = pRenderTarget->CreateSolidColorBrush(vGridColor, &pVGridBrush);
	if ( FAILED(hr)) return;
	hr = pRenderTarget->CreateSolidColorBrush(labelColor, &pLabelBrush);
	if ( FAILED(hr)) return;
	hr = pRenderTarget->CreateSolidColorBrush(textBackgroundColor, &pTextBackgroundBrush);
	if ( FAILED(hr)) return;

  
 //       pRenderTarget->BeginDraw();

	// Get display times and other information
	double highFrequencyLimit = ptheParameterPack->GetFilterHigh();
	double lowFrequencyLimit = ptheParameterPack->GetFilterLow();

	rectangleSize = pRenderTarget->GetSize();
	dataBottom = rc.bottom;
	dataTop = rc.top;

	DrawVerticalGrid( 10, time0, time1 );
	MAPDOUBLE *pLabelMap = ptheParameterPack->GetLabelMap();
	if ( ptheParameterPack->GetScaleChoice() != SCALE_NONE )
			DrawHorizontalGrid( pLabelMap, 10, lowFrequencyLimit, highFrequencyLimit );
    DiscardGraphicsResources();
}

double DrawTimeFrequencyGrid::DrawScaledGridLine( double lastX, double lastFrequency, double newX , double newFrequency, ID2D1SolidColorBrush *pBrush )
{

	// Use the current system scaling	
	newFrequency = ptheFrequencyScale->Scale01( newFrequency );
	lastFrequency = ptheFrequencyScale->Scale01( lastFrequency );

	float fy = (float) (dataBottom - (dataBottom-dataTop) * newFrequency);
	float lastY = (float) (dataBottom - (dataBottom-dataTop) * lastFrequency);
	pRenderTarget->DrawLine( D2D1::Point2F((float) lastX,lastY), D2D1::Point2F((float) newX ,fy), pBrush, 1.0f );

	return lastY;
}

// We have the future option of setting colors and/or alpha
void DrawTimeFrequencyGrid::SetPalette()
{
//	double alpha = 1.0;
	vGridColor =				D2D1::ColorF(1.0f, 0.0f, 0.0f, 0.5f);
	hGridColor =				D2D1::ColorF(1.0f, 0.0f, 0.0f, 0.5f);
	labelColor =				D2D1::ColorF(0.0f, 0.0f, 0.2f, 1.0f);
	textBackgroundColor =	D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.8f);
}

void DrawTimeFrequencyGrid::DrawVerticalGrid( int nVerticalLines, double time0, double time1 )
{
	if ( nVerticalLines <= 0 ) return;

	// Draw vertical gridlines
	for ( int i = 0; i < nVerticalLines; i++ )
	{
		double fraction = (double) i  / ( double ) nVerticalLines;
		float fx = (float) fraction * rectangleSize.width;
		pRenderTarget->DrawLine( D2D1::Point2F(fx, 0.0f), D2D1::Point2F(fx, rectangleSize.height ), pVGridBrush, 1.0f );
		
		CString timeLabel;
		double timeScale = time1 - time0;
		if ( timeScale < 0.01 )
		{
			timeLabel.Format( _T("%-7.4f"), time0 + fraction * (time1 - time0 ) );
		} else if ( timeScale < 0.1 )
		{
			timeLabel.Format( _T("%-6.3f"), time0 + fraction * (time1 - time0 ) );
		}
		else
		{
			timeLabel.Format( _T("%-6.2f"), time0 + fraction * (time1 - time0 ) );
		}
		DrawLabel( timeLabel, fx, 0.0f );
	}
}

// Draw Horizontal lines with labels at places that may be pre-defined
void DrawTimeFrequencyGrid::DrawHorizontalGrid( MAPDOUBLE *pLabelMap, int nHorizontalLines, double lowFrequencyLimit, double highFrequencyLimit )
{
	if ( pLabelMap == nullptr )
	{
		// Must be left to our own devices
		for ( int i = 0; i < nHorizontalLines; i++ )
		{
			float fy;
			double fraction = (double) i  / ( double ) nHorizontalLines;
			fy = (float) ( lowFrequencyLimit + fraction * ( highFrequencyLimit - lowFrequencyLimit) );
			if ( fy > highFrequencyLimit ) break;

			float yCoord = (float) DrawScaledGridLine( 0.0, fy, rectangleSize.width, fy, pVGridBrush );
		
			CString frequencyLabel;
			if ( fy < 0.01 )
				frequencyLabel.Format( _T("%-7.4f"), fy );
			else if ( fy < 1.0 )
				frequencyLabel.Format( _T("%-7.2f"), fy );
			else
				frequencyLabel.Format( _T("%-7.0f"), fy );
			DrawLabel( frequencyLabel, 0.0f, yCoord );
		}
	} else
	{
		MAPDOUBLEIT it;
		for ( it = pLabelMap->begin(); it != pLabelMap->end(); it++ )
		{
			float fy = (float) it->second;
			if ( fy > highFrequencyLimit || fy < lowFrequencyLimit )
				continue;

			float yCoord = (float) DrawScaledGridLine( 0.0, fy, rectangleSize.width, fy, pVGridBrush );
			CString frequencyLabel = it->first;
			DrawLabel( frequencyLabel, 0.0f, yCoord );
		}
	}
}

void DrawTimeFrequencyGrid::DiscardGraphicsResources()
{
	SafeRelease(&pVGridBrush);
	SafeRelease(&pTextBackgroundBrush);
	SafeRelease(&pLabelBrush);
}

void DrawTimeFrequencyGrid::DrawLabel( CString label, float xCoord, float yCoord )
{
	pRenderTarget->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );
	float xLength = 5.0f * fontSize;
//	float vCenter = 1.0f + fontSize/2.0f;

	// Draw background shape
	D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
            D2D1::RectF(xCoord+5.0f, yCoord+4.0f, xCoord + xLength, yCoord + 17.0f), 3.0f, 3.0f );	
	pRenderTarget->FillRoundedRectangle(roundedRect, pTextBackgroundBrush);
    	pRenderTarget->DrawRoundedRectangle(roundedRect, pTextBackgroundBrush, 1.f);

	int len = wcslen( label.GetString() );
	pRenderTarget->DrawTextW( label.GetString(), (UINT32) len, pTextFormat, 
				D2D1::RectF(xCoord+5.0f, yCoord, xCoord + xLength, yCoord + 20.0f), pLabelBrush );

}