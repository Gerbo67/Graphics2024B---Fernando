#include "Canvas.h"
#include <fstream>
#include <Windows.h>

Canvas::Canvas()
{
    m_nSizeX = 0;
    m_nSizeY = 0;
    m_pBuffer = 0;
    m_AddressMode = ADDRESS_MODE_BORDER;
    m_BorderColor = {0, 0, 0, 0};
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
    PIXEL* pPositionixel = m_pBuffer;
    int c = m_nSizeX * m_nSizeY;
    while (c--)
        *pPositionixel++ = color;
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
                  PIXEL color)
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
        // Positionara lineas con m<1
        while (dx--)
        {
            (*this)(x, y) = color;
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
            (*this)(x, y) = color;
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

void Canvas::FillLimits(PIXEL color)
{
    PIXEL* pRow = m_pBuffer;
    for (int j = 0; j < m_nSizeY; j++)
    {
        auto lim = &m_pLimits[j];
        if (lim->left < lim->right)
        {
            int c = lim->right - lim->left;
            PIXEL* pPositionixel = pRow + lim->left;
            while (c--) *pPositionixel++ = color;
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
            PIXEL* pPositionixel = pRow + lim->left;
            while (c--)
            {
                pShader(pPositionixel++, i++, j, x, y);
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
        // Positionara lineas con m<1
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

void Canvas::Circle(int xc, int yc, int r, PIXEL color)
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
        (*this)(x + xc, y + yc) = color;
        (*this)(-x + xc, y + yc) = color;
        (*this)(x + xc, -y + yc) = color;
        (*this)(-x + xc, -y + yc) = color;
        (*this)(y + xc, x + yc) = color;
        (*this)(-y + xc, x + yc) = color;
        (*this)(y + xc, -x + yc) = color;
        (*this)(-y + xc, -x + yc) = color;
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

void Canvas::DrawQuad(VERTEX* pVertex, PIXEL color)
{
    // Lineas horizontales
    Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[1].P.x, pVertex[1].P.y, color);
    Line(pVertex[2].P.x, pVertex[2].P.y, pVertex[3].P.x, pVertex[3].P.y, color);

    // Lineas verticales
    Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[3].P.x, pVertex[3].P.y, color);
    Line(pVertex[1].P.x, pVertex[1].P.y, pVertex[2].P.x, pVertex[2].P.y, color);
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
        Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[1].P.x, pVertex[1].P.y,
             pVertex->color);
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
    for (int i = 0; i < nVertices - 2; i++, pVertex++)
    {
        if ((i & 1) == 0)
        {
            Line(pVertex[2].P.x, pVertex[2].P.y, pVertex[0].P.x, pVertex[0].P.y,
                 pVertex[0].color);
            Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[1].P.x, pVertex[1].P.y,
                 pVertex[1].color);
            Line(pVertex[1].P.x, pVertex[1].P.y, pVertex[2].P.x, pVertex[2].P.y,
                 pVertex[2].color);
        }
        else
        {
            Line(pVertex[2].P.x, pVertex[2].P.y, pVertex[1].P.x, pVertex[1].P.y,
                 pVertex[1].color);
            Line(pVertex[1].P.x, pVertex[1].P.y, pVertex[0].P.x, pVertex[0].P.y,
                 pVertex[0].color);
            Line(pVertex[0].P.x, pVertex[0].P.y, pVertex[2].P.x, pVertex[2].P.y,
                 pVertex[2].color);
        }
    }
}

void Canvas::DrawTriangleFan(VERTEX* pVertex, int nVertices)
{
    VERTEX* pV1 = pVertex + 1;
    VERTEX* pV2 = pVertex + 2;
    for (int i = 0; i < nVertices - 2; i++, pV1++, pV2++)
    {
        Line(pVertex->P.x, pVertex->P.y, pV1->P.x, pV1->P.y, pV1->color);
        Line(pV1->P.x, pV1->P.y, pV2->P.x, pV2->P.y, pV2->color);
        Line(pV2->P.x, pV2->P.y, pVertex->P.x, pVertex->P.y, pVertex->color);
    }
}

bool Canvas::SaveCanvasToFile(const char* pszFileName)
{
    // Guardar nuestro canvas en formato de 24bpp, sin compresión
    fstream out;

    out.open(pszFileName, ios::out | ios::binary);
    if (!out.is_open())
        return false;

    // Creación de las estructuras para manejo del dib
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    memset(&bfh, 0, sizeof(bfh));
    memset(&bih, 0, sizeof(bih));

    // 10*1, 10*24 + 31 = 241 + 31 = 271 / 32 = 8 * 4 = 32 bytes almacenamiento en disco
    int nRowLength = ((m_nSizeX * 24 + 31) / 32) * 4;

    // Computar los datos del encabezado del archivo
    bfh.bfType = 'MB';
    bfh.bfSize = sizeof(bfh) + sizeof(bih) + nRowLength * m_nSizeY + 0; // 0 corresponde al tamaño de la paleta
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih);

    // Computar la información de la imagen
    bih.biSize = sizeof(bih); // Tamaño del header de la imagen
    bih.biWidth = m_nSizeX;
    bih.biHeight = m_nSizeY;
    bih.biPlanes = 1;
    bih.biBitCount = 24; //bpp
    bih.biCompression = BI_RGB;
    bih.biSizeImage = nRowLength * m_nSizeY; // Tamaño en disco
    bih.biXPelsPerMeter = 1000;
    bih.biYPelsPerMeter = 1000;

    // Escribir los encabezados en el archivo
    out.write((char*)&bfh, sizeof(bfh));
    out.write((char*)&bih, sizeof(bih));

    // Crear y guardar la paleta segun aplique
    switch (bih.biBitCount)
    {
    case 1:
    case 4:
    case 8:
        // Construir y guardar la paleta
        break;
    }

    // Guardar la imagen
    char* pRowBuffer = new char[nRowLength];
    memset(pRowBuffer, 0, nRowLength);
    switch (bih.biBitCount)
    {
    case 24:
        for (int j = m_nSizeY - 1; j >= 0; j--)
        {
            PIXEL* pSource = &(*this)(0, j);
            RGBTRIPLE* pEncoding = (RGBTRIPLE*)pRowBuffer;
            for (int i = 0; i < m_nSizeX; i++)
            {
                pEncoding->rgbtRed = pSource->r;
                pEncoding->rgbtGreen = pSource->g;
                pEncoding->rgbtBlue = pSource->b;

                pEncoding++;
                pSource++;
            }

            out.write(pRowBuffer, nRowLength);
        }
    }

    delete[] pRowBuffer;
    out.close();
    return true;
}

Canvas* Canvas::Clone()
{
    Canvas* pCanvas = CreateCanvas(m_nSizeX, m_nSizeY);
    memcpy(pCanvas->m_pBuffer, m_pBuffer, m_nSizeX * m_nSizeY * sizeof(PIXEL));
    return pCanvas;
}

 int Canvas::CalculateShift(DWORD mask)
 {
    if (mask == 0) return 0;
    int shift = 0;
    while ((mask & 1) == 0) {
        mask >>= 1;
        shift++;
    }
    return shift;
 }

Canvas* Canvas::CreateCanvasFromFile(const char* pszFileName, PIXEL (*pfLoadPixel)(PIXEL Color))
{
    // Cargar nuestro canvas en formato de 8bpp y 24bpp, sin compresión
    fstream in;

    in.open(pszFileName, ios::in | ios::binary);
    if (!in.is_open())
        return nullptr;

    // Creación de las estructuras para manejo del dib
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    memset(&bfh, 0, sizeof(bfh));
    memset(&bih, 0, sizeof(bih));

    // Leer los encabezados del archivo
    in.read((char*)&bfh, sizeof(bfh));

    // Validar que el archivo que estamos procesandoes un archivo dip, bitmap
    if (bfh.bfType != 'MB')
        return nullptr;

    // Leer los encabezados de la imagen
    in.read((char*)&bih, sizeof(bih));
    if (bih.biSize != sizeof(bih))
        return nullptr;

    // Computo de rowlength
    // 10*1, 10*24 + 31 = 241 + 31 = 271 / 32 = 8 * 4 = 32 bytes almacenamiento en disco
    int nRowLength = ((bih.biWidth * bih.biBitCount + 31) / 32) * 4;

    // Cargar la paleta segun aplique
    RGBQUAD palette[256];
    int nColorsToRead = 0;

    switch (bih.biBitCount)
    {
    case 1:
    case 4:
    case 8:
        // Construir y guardar la paleta
        nColorsToRead = bih.biClrUsed ? bih.biClrUsed : (1 << bih.biBitCount);
        break;
    }
    in.read((char*)&palette, nColorsToRead * sizeof(RGBQUAD));

    // Cargar la imagen
    Canvas* pCanvas = CreateCanvas(bih.biWidth, bih.biHeight);

    char* pRowBuffer = new char[nRowLength]; // Buffer de lectura linea por linea del archivo
    memset(pRowBuffer, 0, nRowLength);
    switch (bih.biBitCount)
    {
    case 8:
        for (int j = bih.biHeight - 1; j >= 0; j--)
        {
            PIXEL* pDest = &(*pCanvas)(0, j);
            unsigned char* pDecode = (unsigned char*)pRowBuffer;
            in.read((char*)pRowBuffer, nRowLength);
            for (int i = 0; i < bih.biWidth; i++)
            {
                pDest->r = palette[*pDecode].rgbRed;
                pDest->g = palette[*pDecode].rgbGreen;
                pDest->b = palette[*pDecode].rgbBlue;
                pDest->a = 0xFF;

                if (pfLoadPixel)
                    *pDest = pfLoadPixel(*pDest);

                pDecode++;
                pDest++;
            }
        }
        break;
    case 24:
        for (int j = bih.biHeight - 1; j >= 0; j--)
        {
            PIXEL* pDest = &(*pCanvas)(0, j);
            RGBTRIPLE* pDecode = (RGBTRIPLE*)pRowBuffer;
            in.read((char*)pRowBuffer, nRowLength);
            for (int i = 0; i < bih.biWidth; i++)
            {
                pDest->r = pDecode->rgbtRed;
                pDest->g = pDecode->rgbtGreen;
                pDest->b = pDecode->rgbtBlue;
                pDest->a = 0xFF;

                if (pfLoadPixel)
                    *pDest = pfLoadPixel(*pDest);

                pDecode++;
                pDest++;
            }
        }
        break;
    }

    delete[] pRowBuffer;
    in.close();
    return pCanvas;
}

Canvas::PIXEL Canvas::Peek(int i, int j)
{
    int p, q;
    switch (m_AddressMode)
    {
    case ADDRESS_MODE_BORDER:
        if (i >= 0 && i < m_nSizeX &&
            j >= 0 && j < m_nSizeY)
            return m_pBuffer[i + j * m_nSizeX];
        else
            return m_BorderColor;
    case ADDRESS_MODE_CLAMP:
        i = min(max(0,i), m_nSizeX - 1);
        j = min(max(0,j), m_nSizeY - 1);
        return m_pBuffer[i + j * m_nSizeX];
    case ADDRESS_MODE_WRAP:
        i %= m_nSizeX;
        j %= m_nSizeY;
        if (i < 0) i = m_nSizeX + i;
        if (j < 0) j = m_nSizeY + j;
        return m_pBuffer[i + j * m_nSizeX];
    case ADDRESS_MODE_MIRROR:
        p = i / m_nSizeX; // Observar en que repetición estoy, en i.
        q = j / m_nSizeY;

        i = abs(i % m_nSizeX); // Determinando que parte de la textura aplicar, sin signo.
        j = abs(j % m_nSizeY);

        i = (p & 1) ? (m_nSizeX - 1) - i : i; // Tomar la parte de la textura que toca, dependiendo el valor de p y q.
        j = (q & 1) ? (m_nSizeY - 1) - j : j;
    // Valores pares muestran la textura en orientación normal, impares, en orientación espejo.

        return m_pBuffer[i + j * m_nSizeX];
    }
    return m_BorderColor;
}

void Canvas::SetAddressMode(AddressMode Mode)
{
    m_AddressMode = Mode;
}

void Canvas::SetColorBorder(PIXEL color)
{
    m_BorderColor = color;
}

Canvas::PIXEL Canvas::PointSampler(float s, float t)
{
    return Peek((int)floorf(s + 0.5f), (int)floorf(t + 0.5f));
}

Canvas::PIXEL Canvas::BilinearSampler(float s, float t)
{
    PIXEL A, B, C, D;
    int p, q;
    float f, g;
    int j, k;

    // Calcular el textel pivote
    p = (int)floorf(s);
    q = (int)floorf(t);

    // Obtener los texeles adyacentes a (s, t)
    A = Peek(p, q);
    B = Peek(p + 1, q);
    C = Peek(p, q + 1);
    D = Peek(p + 1, q + 1);

    // Fracción interpolante
    f = s - p;
    g = t - q;
    j = 256 * f;
    k = 256 * g;

    return Lerp(Lerp(A, B, j), Lerp(C, D, j), k);
}

void Canvas::TextureInverseMapping(const VERTEX V[3], Canvas* pTexture)
{
    float A, B, C, D, E, F, det;
    det = V[0].P.x * (V[1].P.y - V[2].P.y) +
        V[0].P.y * (V[2].P.x - V[1].P.x) +
        (V[1].P.x * V[2].P.y - V[2].P.x * V[1].P.y);
    if (det < 0) return;
    A = (V[1].TexCoord.x * V[2].P.y -
            V[2].TexCoord.x * V[1].P.y) +
        (V[2].TexCoord.x * V[0].P.y -
            V[0].TexCoord.x * V[2].P.y) +
        (V[0].TexCoord.x * V[1].P.y -
            V[1].TexCoord.x * V[0].P.y);
    B = (V[1].P.x * V[2].TexCoord.x -
            V[2].P.x * V[1].TexCoord.x) +
        (V[2].P.x * V[0].TexCoord.x -
            V[0].P.x * V[2].TexCoord.x) +
        (V[0].P.x * V[1].TexCoord.x -
            V[1].P.x * V[0].TexCoord.x);
    C = V[0].P.x * (
            V[1].P.y * V[2].TexCoord.x -
            V[2].P.y * V[1].TexCoord.x) +
        V[0].P.y * (
            V[2].P.x * V[1].TexCoord.x -
            V[1].P.x * V[2].TexCoord.x) +
        V[0].TexCoord.x * (
            V[1].P.x * V[2].P.y -
            V[2].P.x * V[1].P.y);

    D = (V[1].TexCoord.y * V[2].P.y -
            V[2].TexCoord.y * V[1].P.y) +
        (V[2].TexCoord.y * V[0].P.y -
            V[0].TexCoord.y * V[2].P.y) +
        (V[0].TexCoord.y * V[1].P.y -
            V[1].TexCoord.y * V[0].P.y);
    E = (V[1].P.x * V[2].TexCoord.y -
            V[2].P.x * V[1].TexCoord.y) +
        (V[2].P.x * V[0].TexCoord.y -
            V[0].P.x * V[2].TexCoord.y) +
        (V[0].P.x * V[1].TexCoord.y -
            V[1].P.x * V[0].TexCoord.y);
    F = V[0].P.x * (
            V[1].P.y * V[2].TexCoord.y -
            V[2].P.y * V[1].TexCoord.y) +
        V[0].P.y * (
            V[2].P.x * V[1].TexCoord.y -
            V[1].P.x * V[2].TexCoord.y) +
        V[0].TexCoord.y * (
            V[1].P.x * V[2].P.y -
            V[2].P.x * V[1].P.y);
    det = 1.0f / det;
    //Se calcula el recíproco del determinante, para optimizar su aplicación con los coeficientes
    //Se multiplica por el tamaño de la textura, para cambiarlo del espacio normalizado u,v; al espacio s,t
    A *= det * (pTexture->GetSizeX() - 1);
    B *= det * (pTexture->GetSizeX() - 1);
    C *= det * (pTexture->GetSizeX() - 1);
    D *= det * (pTexture->GetSizeY() - 1);
    E *= det * (pTexture->GetSizeY() - 1);
    F *= det * (pTexture->GetSizeY() - 1);
    //Se establecen los límites de la geometría
    ResetLimits();
    LineLimits(V[0].P.x + 0.5f, V[0].P.y + 0.5f, V[1].P.x + 0.5f, V[1].P.y + 0.5f);
    LineLimits(V[1].P.x + 0.5f, V[1].P.y + 0.5f, V[2].P.x + 0.5f, V[2].P.y + 0.5f);
    LineLimits(V[2].P.x + 0.5f, V[2].P.y + 0.5f, V[0].P.x + 0.5f, V[0].P.y + 0.5f);
    int y;
    int cx;
    //Se busca el primer límite a rellenar.
    for (y = 0; y < m_nSizeY; y++)
    {
        cx = m_pLimits[y].right - m_pLimits[y].left;
        if (cx > 0) break;
    }

    //Se aplica la matriz de transformación de manera similar cómo lo hicimos en la versión optimizada para cada vértice
    float byc = B * y + C;
    float eyc = E * y + F;
    PIXEL* pLine = m_pBuffer + y * m_nSizeX;
    for (; y < m_nSizeY; y++)
    {
        cx = m_pLimits[y].right - m_pLimits[y].left;
        if (cx < 0) break;
        int x = m_pLimits[y].left;
        PIXEL* pWrite = pLine + x;
        float s = A * x + byc;
        float t = D * x + eyc;
        byc += B;
        eyc += E;
        while (cx--)
        {
            *pWrite++ = pTexture->BilinearSampler(s, t);
            s += A;
            t += D;
        }
        pLine += m_nSizeX;
    }
}

void Canvas::TextureMappingQuad(const VERTEX V[4], Canvas* pTexture)
{
    // Definir las dos mitades del cuadrado como dos triángulos
    VERTEX Triangle1[] = {V[0], V[1], V[2]};
    VERTEX Triangle2[] = {V[0], V[2], V[3]};

    // Llamar a la función de mapeo para cada triángulo
    TextureInverseMapping(Triangle1, pTexture);
    TextureInverseMapping(Triangle2, pTexture);
}


