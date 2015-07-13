
// DrawTimeFrequencyGrid.h : interface of the DrawTimeFrequencyGrid class
//

#pragma once
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "ParameterPack.h"

class DrawTimeFrequencyGrid 
{
public:
	DrawTimeFrequencyGrid();
	~DrawTimeFrequencyGrid();

// Operations

public:
	void Draw( ID2D1RenderTarget *pRenderTarget, D2D1_RECT_F rc, ParameterPack *ptheParameterPack, double time0, double time1 ); 
	BOOL Initialize( float fontSize );

private:
	float fontSize;
	ID2D1RenderTarget *pRenderTarget;
	FrequencyScale	*ptheFrequencyScale;

// Graphics
private:

	// palette
	D2D1_COLOR_F vGridColor;
	D2D1_COLOR_F hGridColor;
	D2D1_COLOR_F labelColor;
	D2D1_COLOR_F textBackgroundColor;

    ID2D1SolidColorBrush    *pLabelBrush;
    ID2D1SolidColorBrush    *pVGridBrush;
    ID2D1SolidColorBrush    *pTextBackgroundBrush;

	ID2D1Factory            *pFactory;
	IDWriteFactory			*pDWriteFactory;
	IDWriteTextFormat		*pTextFormat;

// Bounding and scaling information
private:
	// Main drawing rectangle
	D2D1_SIZE_F rectangleSize;

	// Data view
	float dataBottom;
	float dataTop;

    void    DiscardGraphicsResources();

	void SetPalette();
	void DrawVerticalGrid(  int nVerticalLines, double time0, double time1 );
	void DrawHorizontalGrid( MAPDOUBLE *pLabelMap, int nHorizontalLines, double frequency0, double frequency1 );
	double DrawScaledGridLine(double lastx, double lastVal, double fx , double newVal, ID2D1SolidColorBrush *pBrush );
	void DrawLabel( CString label, float yCoord, float xCoord  );
};
