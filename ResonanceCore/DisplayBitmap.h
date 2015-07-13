
// DisplayBitmap.h : interface of the DisplayBitmap class
// Holds the main specialized graphics render display code.
//

#pragma once
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "ParameterPack.h"
#include "DrawTimeFrequencyGrid.h"
#include <wincodec.h>

class Coordinator;

class DisplayBitmap
{
public:
	DisplayBitmap( Coordinator* parent );
	~DisplayBitmap();
	BOOL Initialize( UINT bitmapHeight, UINT bitmapWidth );
	BOOL ReSizeBitmap();
	BOOL Release();
	BOOL onSaveDisplayAs( CString pathname );

private:
	Coordinator *pTheCoordinator;
	ParameterPack *ptheParameterPack;

// Graphics
private:
	vector<ID2D1SolidColorBrush *>   brushSpectrumPalette;

	ID2D1Factory            *pFactory;
//	IDWriteFactory			*pDWriteFactory;

	// Main bitmap resources
	IWICImagingFactory *pWICFactory;
	IWICBitmap *pWICBitmap;
//	IWICBitmapLock *pLock;
	UINT bitmapWidth;
	UINT bitmapHeight;
    ID2D1RenderTarget   *pBitmapRenderTarget;
	IWICBitmapEncoder	*pBitmapEncoder;
	ID2D1Bitmap			*pTitleBitmap;

	DrawTimeFrequencyGrid *ptheDrawTimeFrequencyGrid;

	FVECTOR workingBuffer;
	FVECTOR workingBuffer2;

private:
	double time0;
	double time1;
	float brushHeight;
	float brushWidth;
	double scaleBreak;

	void SetSpectrumPalette( double alpha, int interpolationLevels );
	void Paint( float intensity, float fx, float fy );
	double scaler( double x ) { return log10( x ); };
	//double scaler( double x ) { return sqrt(x); };
	void ContrastEnhance( double controlValue, FVECTOR *pIn, FVECTOR *pOut );
	void PasteLogo();

	HRESULT LoadResourceBitmap(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		int resourceID, LPCTSTR resourceType,
//		UINT destinationWidth,
//		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap
		);



public:
	IWICBitmap* RenderSpectrumBitmap( double displayLowRange, double displayHighRange, double peakEnhanceValue, double time0, double time1 );
};
