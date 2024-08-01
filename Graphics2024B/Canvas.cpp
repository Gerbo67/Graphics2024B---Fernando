#include "Canvas.h"

Canvas::Canvas()
{
    m_nSizeX = 0;
    m_nSizeY = 0;
    m_pBuffer = 0;
}

Canvas::~Canvas()
{
}

Canvas* Canvas::CreateCanvas(int nSizeX, int nSizeY)
{
    Canvas* pNewCanvas = new Canvas();
    pNewCanvas->m_pBuffer = new PIXEL[nSizeX * nSizeY];
    pNewCanvas->m_nSizeX = nSizeX;
    pNewCanvas->m_nSizeY = nSizeY;
    pNewCanvas->m_pLimits = new LIMIT[nSizeY];
    return pNewCanvas;
}

void Canvas::DestroyCanvas(Canvas* pDestroy)
{
    delete[] pDestroy->m_pBuffer;
    delete[] pDestroy->m_pLimits;
    delete pDestroy;
}

Canvas::PIXEL& Canvas::operator()(int i, int j)
{
    static PIXEL dummy;
    if (i >= 0 && i < m_nSizeX &&
        j >= 0 && j < m_nSizeY)
        return m_pBuffer[i + j * m_nSizeX];
    return dummy;
}

void Canvas::Clear(PIXEL color)
{
    PIXEL* pPixel = m_pBuffer;
    int c = m_nSizeX * m_nSizeY;
    while (c--)
        *pPixel++ = color;
}

Canvas::PIXEL Canvas::Lerp(PIXEL A, PIXEL B, short c)
{
    /*  x(t) = x0 + t(x1-x0) */

    /*
    *     0<c<255 , usando 8 bits una mantisa fija de un numero fraccional
    *     
    *     t = c / 255
    *     c = 255*t
    * 
    *     XXXX XXXX *
    *     YYYY YYYY
    * ----------------
    * ZZZh ZZZh ZZZl ZZZl >> 8
    * ----------------
    * ZZZh ZZZh */
    return {
        (unsigned char)(A.r + (((c * ((short)B.r - A.r)) >> 8))),
        (unsigned char)(A.g + (((c * ((short)B.g - A.g)) >> 8))),
        (unsigned char)(A.b + (((c * ((short)B.b - A.b)) >> 8))),
        (unsigned char)(A.a + (((c * ((short)B.a - A.a)) >> 8)))
    };
}

void Canvas::Shade(SHADER pShader)
{
    PIXEL* pDest = m_pBuffer;
    int dx, dy;
    dx = (1 << 16) / m_nSizeX;
    dy = (1 << 16) / m_nSizeY;
    int y = 0;
    for (int j = 0; j < m_nSizeY; j++)
    {
        int x = 0;
        for (int i = 0; i < m_nSizeX; i++)
        {
            pShader(pDest++, i, j, x, y);
            x += dx;
        }
        y += dy;
    }
}

void Canvas::Line(int x0, int y0, int x1, int y1,
                  PIXEL Color)
{
    int dx, dy, p, x, y;
    x = x0;
    y = y0;
    dx = x1 - x0;
    dy = y1 - y0;
    int incx = dx >= 0 ? 1 : -1;
    int incy = dy >= 0 ? 1 : -1;
    dx = dx < 0 ? -dx : dx;
    dy = dy < 0 ? -dy : dy;
    // m <= 1
    if (dy <= dx)
    {
        p = dx - 2 * dy;
        int dp0 = -2 * dy;
        int dp1 = 2 * dx - 2 * dy;
        // Para lineas con m<1
        while (dx--)
        {
            (*this)(x, y) = Color;
            if (p > 0)
            {
                p += dp0;
            }
            else
            {
                p += dp1;
                y += incy;
            }
            x += incx;
        }
    }
    else // m > 1    x <---> y
    {
        p = dy - 2 * dx;
        int dp0 = -2 * dx;
        int dp1 = 2 * dy - 2 * dx;
        while (dy--)
        {
            (*this)(x, y) = Color;
            if (p > 0)
            {
                p += dp0;
            }
            else
            {
                p += dp1;
                x += incx;
            }
            y += incy;
        }
    }
}

void Canvas::ResetLimits()
{
    /* Una linea no se rellena si left > right */
    for (int j = 0; j < m_nSizeY; j++)
    {
        m_pLimits[j].left = m_nSizeX;
        m_pLimits[j].right = -1;
    }
}

#include <algorithm>
using namespace std;

void Canvas::SetLimit(int i, int j)
{
    if (j >= 0 && j < m_nSizeY)
    {
        auto lim = &m_pLimits[j];
        if (lim->left > i)
            lim->left = max(0, i);
        if (lim->right < i)
            lim->right = min(m_nSizeX, i);
    }
}

void Canvas::FillLimits(PIXEL Color)
{
    PIXEL* pRow = m_pBuffer;
    for (int j = 0; j < m_nSizeY; j++)
    {
        auto lim = &m_pLimits[j];
        if (lim->left < lim->right)
        {
            int c = lim->right - lim->left;
            PIXEL* pPixel = pRow + lim->left;
            while (c--) *pPixel++ = Color;
        }
        pRow += m_nSizeX;
    }
}

void Canvas::FillLimits(SHADER pShader)
{
    int x, y;
    PIXEL* pRow = m_pBuffer;
    int dx = (1 << 16) / m_nSizeX;
    int dy = (1 << 16) / m_nSizeY;
    y = 0;
    for (int j = 0; j < m_nSizeY; j++)
    {
        auto lim = &m_pLimits[j];
        if (lim->left < lim->right)
        {
            int i = lim->left;
            x = i * dx;
            int c = lim->right - lim->left;
            PIXEL* pPixel = pRow + lim->left;
            while (c--)
            {
                pShader(pPixel++, i++, j, x, y);
                x += dx;
            }
        }
        pRow += m_nSizeX;
        y += dy;
    }
}

void Canvas::LineLimits(int x0, int y0, int x1, int y1)
{
    int dx, dy, p, x, y;
    x = x0;
    y = y0;
    dx = x1 - x0;
    dy = y1 - y0;
    int incx = dx >= 0 ? 1 : -1;
    int incy = dy >= 0 ? 1 : -1;
    dx = dx < 0 ? -dx : dx;
    dy = dy < 0 ? -dy : dy;
    // m <= 1
    if (dy <= dx)
    {
        p = dx - 2 * dy;
        int dp0 = -2 * dy;
        int dp1 = 2 * dx - 2 * dy;
        // Para lineas con m<1
        while (dx--)
        {
            this->SetLimit(x, y);
            if (p > 0)
            {
                p += dp0;
            }
            else
            {
                p += dp1;
                y += incy;
            }
            x += incx;
        }
    }
    else // m > 1    x <---> y
    {
        p = dy - 2 * dx;
        int dp0 = -2 * dx;
        int dp1 = 2 * dy - 2 * dx;
        while (dy--)
        {
            this->SetLimit(x, y);
            if (p > 0)
            {
                p += dp0;
            }
            else
            {
                p += dp1;
                x += incx;
            }
            y += incy;
        }
    }
}

void Canvas::Circle(int xc, int yc, int r, PIXEL Color)
{
    //f(x,y) = x^2 + y^2 - r^2  = 0
    int x = 0;
    int y = r;
    int p = 5 - 4 * r;
    int _8x, _8y;
    _8x = 0;
    _8y = 8 * y;
    while (x < y)
    {
        (*this)(x + xc, y + yc) = Color;
        (*this)(-x + xc, y + yc) = Color;
        (*this)(x + xc, -y + yc) = Color;
        (*this)(-x + xc, -y + yc) = Color;
        (*this)(y + xc, x + yc) = Color;
        (*this)(-y + xc, x + yc) = Color;
        (*this)(y + xc, -x + yc) = Color;
        (*this)(-y + xc, -x + yc) = Color;
        if (p > 0)
        {
            p += _8x - _8y + 20;
            y--;
            _8y -= 8;
        }
        else
            p += _8x + 12;
        _8x += 8;
        x++;
    }
}

void Canvas::CircleLimits(int xc, int yc, int r)
{
    //f(x,y) = x^2 + y^2 - r^2  = 0
    int x = 0;
    int y = r;
    int p = 5 - 4 * r;
    int _8x, _8y;
    _8x = 0;
    _8y = 8 * y;
    while (x < y)
    {
        SetLimit(x + xc, y + yc);
        SetLimit(-x + xc, y + yc);
        SetLimit(x + xc, -y + yc);
        SetLimit(-x + xc, -y + yc);
        SetLimit(y + xc, x + yc);
        SetLimit(-y + xc, x + yc);
        SetLimit(y + xc, -x + yc);
        SetLimit(-y + xc, -x + yc);
        if (p > 0)
        {
            p += _8x - _8y + 20;
            y--;
            _8y -= 8;
        }
        else
            p += _8x + 12;
        _8x += 8;
        x++;
    }
}

void Canvas::VertexProcessor(void* ctx, VERTEXSHADER pVS, VERTEX* pInput, VERTEX* pOutput, int nVertices)
{
    for (int i = 0; i < nVertices; i++)
    {
        pVS(ctx, *pInput++, *pOutput++);
    }
}

void Canvas::DrawTriangleList(VERTEX* pVertex, int nVertices, PIXEL color)
{
    for (int i = 0; i < nVertices; i += 3)
    {
        VERTEX* pT = pVertex;
        Line(pT[0].P.x, pT[0].P.y, pT[1].P.x, pT[1].P.y, color);
        Line(pT[1].P.x, pT[1].P.y, pT[2].P.x, pT[2].P.y, color);
        Line(pT[2].P.x, pT[2].P.y, pT[0].P.x, pT[0].P.y, color);
        pVertex += 3;
    }
}

void Canvas::DrawPointList(VERTEX* pVertex, int nVertices)
{
    for (int i = 0; i < nVertices; i++, pVertex++)
        operator()(pVertex->P.x, pVertex->P.y) = pVertex->color;
}

void Canvas::DrawLineList(VERTEX* pVertex, int nVertices)
{
    for (int i = 0; i < nVertices - 1; i += 2, pVertex += 2)
    {
        Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[1].P.x, pVertex[1].P.y, pVertex->color);
    }
}

void Canvas::DrawLineStrip(VERTEX* pVertex, int nVertices)
{
    for (int i = 0; i < nVertices - 1; i++)
    {
        VERTEX* pV = pVertex;
        pVertex++;
        Line(pV->P.x, pV->P.y, pVertex->P.x, pVertex->P.y, pV->color);
    }
}

void Canvas::DrawTriangleStrip(VERTEX* pVertex, int nVertices)
{
    for (int i = 0; i < nVertices -2; i++, pVertex++)
    {
        if((i & 1) == 0)
        {
            Line(pVertex[2].P.x, pVertex[2].P.y, pVertex[0].P.x, pVertex[0].P.y, pVertex[0].color);
            Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[1].P.x, pVertex[1].P.y, pVertex[1].color);
            Line(pVertex[1].P.x, pVertex[1].P.y, pVertex[2].P.x, pVertex[2].P.y, pVertex[2].color);
        }
        else
        {
            Line(pVertex[2].P.x, pVertex[2].P.y, pVertex[1].P.x, pVertex[1].P.y, pVertex[1].color);
            Line(pVertex[1].P.x, pVertex[1].P.y, pVertex[0].P.x, pVertex[0].P.y, pVertex[0].color);
            Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[2].P.x, pVertex[2].P.y, pVertex[2].color);
        }
    }
}

void Canvas::DrawTriangleFan(VERTEX* pVertex, int nVertices)
{
    VERTEX *pV1 = pVertex + 1;
    VERTEX *pV2 = pVertex + 2;
    for(int i = 0; i < nVertices - 2; i++, pV1++, pV2++)
    {
        Line(pVertex->P.x, pVertex->P.y, pV1->P.x, pV1->P.y, pV1->color);
        Line(pV1->P.x, pV1->P.y, pV2->P.x, pV2->P.y, pV2->color);
        Line(pV2->P.x, pV2->P.y, pVertex->P.x, pVertex->P.y, pVertex->color);
    }
}
