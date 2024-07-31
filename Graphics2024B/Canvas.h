#pragma once
#include "Matrix4D.h"

class Canvas
{
public:
	union PIXEL
	{
		struct
		{
			unsigned char r, g, b, a;
		};
		struct
		{
			unsigned char c, m, y, b;
		};
		long p;
	};

	struct VERTEX
	{
		VECTOR4D P;
	};

	struct Vertex { float x, y, z, w; };
protected:
	struct LIMIT
	{
		int left;
		int right;
	};
	int m_nSizeX; // Tama�o horizontal del canvas en p�xeles
	int m_nSizeY; // Tama�o vertical del canvas en p�xeles
	PIXEL* m_pBuffer; //Buffer de pixeles
	LIMIT* m_pLimits; //Buffer de limites de relleno
	Canvas();
	~Canvas();
public:
	static Canvas* CreateCanvas(int nSizeX, int nSizeY);
	static void DestroyCanvas(Canvas* pDestroy);
	PIXEL& operator()(int i, int j);
	void Clear(PIXEL color);
	void* GetBuffer() { return m_pBuffer; };
	unsigned int GetPitch() { return sizeof(PIXEL)* m_nSizeX; }
	int GetSizeX() { return m_nSizeX; }
	int GetSizeY() { return m_nSizeY; }
	static PIXEL Lerp(PIXEL A, PIXEL B, short c);
	typedef void (*SHADER)(PIXEL* pDest, int i, int j, 
		          int x, int y);
	void Shade(SHADER pShader);
	void Line(int x0, int y0, int x1, int y1,
		      PIXEL Color);
	void ResetLimits();
	void SetLimit(int i, int j);
	void FillLimits(PIXEL Color);
	void FillLimits(SHADER pShader);
	void LineLimits(int x0, int y0, int x1, int y1);
	void Circle(int xc, int yc, int r, PIXEL Color);
	void CircleLimits(int xc, int yx, int r);

	// Procesador de vertices
	typedef void(*VERTEXSHADER) (void* ctx, VERTEX& Input, VERTEX& Output);
	static void VertexProcessor(void* ctx, VERTEXSHADER pVS, VERTEX* pInput, VERTEX* pOutput, int nVertices);
	void DrawTriangleList(VERTEX* pVertex, int nVertices, PIXEL color);
	
	int GetWidth() const {
		return m_nSizeX;
	}

	int GetHeight() const {
		return m_nSizeY;
	}
};



