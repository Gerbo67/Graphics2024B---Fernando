#include "Application.h"
#include "Canvas.h"
#include "Matrix4D.h"
#include <stdio.h>
#include <time.h>
#include <vector>
Application* Application::sm_pApp;

Application* Application::GetApplication()
{
    if (!sm_pApp)
        sm_pApp = new Application();
    return sm_pApp;
}

Application::Application()
{
    //Initialize application data!!!
    m_pLastFrame = nullptr;
    m_pImage = nullptr;
}

Application::~Application()
{
    //Uninitalize application data
}

void Application::Run()
{
    MSG msg;
    if (!Initialize())
    {
        MessageBox(NULL,
                   TEXT("Error al inicializar"),
                   TEXT("Application::Run()"),
                   MB_ICONERROR);
        return;
    }
    ShowWindow(m_hWnd, m_nCmdShow);
    while (1)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                break;
        }
        Update();
    }
    Uninitialize();
}

bool Application::Initialize()
{
    m_pAdapter = m_DXGIManager.ChooseAdapter();
    if (m_pAdapter)
    {
        if (!m_DXGIManager.InitializeSwapChain(m_pAdapter))
            return false;
    }
    else
        return false;
    return true;
}

bool Application::Uninitialize()
{
    if (m_pLastFrame) Canvas::DestroyCanvas(m_pLastFrame);
    if (m_pImage) Canvas::DestroyCanvas(m_pLastFrame);
    return true;
}

void ShaderCircle(Canvas::PIXEL* pDest, int i, int j,
                  int unused_x, int unused_y)
{
    /* r^2 > (yc-y)^2 + (xc-x)^2 */
    int x, y, r = 100 * 100;
    x = 400 - i;
    y = 400 - j;
    x *= x;
    y *= y;
    if (r > (x + y))
        *pDest = {255, 255, 255, 0};
    else
        *pDest = {255, 0, 0, 0};
}

void ShaderChess(Canvas::PIXEL* pDest,
                 int i, int j, int x, int y)
{
    int p, q;
    Canvas::PIXEL Black = {0, 0, 0, 0}, White = {255, 255, 255, 0};
    p = i / 16;
    q = j / 16;
    *pDest = (p & 1) ^ (q & 1) ? Black : White;
}

void ShaderColors(Canvas::PIXEL* pDest,
                  int i, int j, int x, int y)
{
    Canvas::PIXEL
        A = {255, 0, 0, 0},
        B = {0, 255, 0, 0},
        C = {255, 255, 0, 0},
        D = {0, 0, 255, 0};
    *pDest = Canvas::Lerp(
        Canvas::Lerp(A, B, x >> 8),
        Canvas::Lerp(C, D, x >> 8), y >> 8);
}

void ShaderNoise(Canvas::PIXEL* pDest,
                 int i, int j, int x, int y)
{
    unsigned char c = (unsigned char)rand();
    *pDest = {c, c, c, c}; // Generamos un pixel en escala de grises, Aleatorio.
}

void VertexShaderSimple(MATRIX4D* pM, Canvas::VERTEX& Input, Canvas::VERTEX& Output)
{
    Output.P = Input.P * (*pM);
}

void ListPoints(Canvas* pCanvas)
{
    Canvas::VERTEX Points[] =
    {
        {{100, 100, 0, 1}, {255, 255, 255, 0}},
        {{150, 150, 0, 1}, {255, 0, 0, 0}},
        {{100, 200, 0, 1}, {0, 255, 0, 0}},
        {{50, 150, 0, 1}, {0, 0, 255, 0}},
    };
    pCanvas->DrawPointList(Points, 4);
}

void ListLine(Canvas* pCanvas, float time)
{
    static int t = 0;
    Canvas::VERTEX Lines[] =
    {
        {{100, 0, 0, 1}, {255, 255, 255, 0}},
        {{100, 30, 0, 1}, {255, 255, 255, 0}},
        {{120, 20, 0, 1}, {255, 255, 255, 0}},
        {{120, 50, 0, 1}, {255, 255, 255, 0}},
        {{130, 5, 0, 1}, {255, 255, 255, 0}},
        {{130, 35, 0, 1}, {255, 255, 0, 0}},
        {{150, 25, 0, 1}, {255, 255, 255, 0}},
        {{150, 60, 0, 1}, {255, 255, 0, 0}},
    };
    Canvas::VERTEX OutputLines[8];
    MATRIX4D T = Translation(0, t, 0);
    Canvas::VertexProcessor(&T, (Canvas::VERTEXSHADER)VertexShaderSimple, Lines, OutputLines, 8);
    pCanvas->DrawLineList(OutputLines, 8);
    t += 5;
}

void StripLines(Canvas* pCanvas)
{
    Canvas::VERTEX Lines[] =
    {
        {{100, 0, 0, 1}, {255, 255, 255, 0}},
        {{100, 30, 0, 1}, {255, 255, 255, 0}},
        {{120, 20, 0, 1}, {255, 255, 255, 0}},
        {{120, 50, 0, 1}, {255, 255, 255, 0}},
        {{130, 5, 0, 1}, {255, 255, 255, 0}},
        {{130, 35, 0, 1}, {255, 255, 0, 0}},
        {{150, 25, 0, 1}, {255, 255, 255, 0}},
        {{150, 60, 0, 1}, {255, 255, 0, 0}},
    };
    pCanvas->DrawLineStrip(Lines, 8);
}

void StripTriangles(Canvas* pCanvas)
{
    Canvas::VERTEX StripTri[] =
    {
        {{-1, -1, 0, 1}, {255, 255, 255, 0}},
        {{0, 1, 0, 1}, {255, 255, 255, 0}},
        {{1, -1, 0, 1}, {255, 255, 255, 0}},
        {{2, 1, 0, 1}, {255, 255, 255, 0}},
        {{3, -1, 0, 1}, {255, 255, 255, 0}},
        {{4, 1, 0, 1}, {255, 255, 255, 0}},
        {{5, -1, 0, 1}, {255, 255, 255, 0}},
    };
    Canvas::VERTEX OutputStripTri[7];

    MATRIX4D ST = Scaling(50, 50, 1) * RotationZ((float)3.141592f / 8) * Translation(300, 300, 0);
    Canvas::VertexProcessor(&ST, (Canvas::VERTEXSHADER)VertexShaderSimple, StripTri, OutputStripTri, 7);

    pCanvas->DrawTriangleStrip(OutputStripTri, 7);
}

void FanTriangles(Canvas* pCanvas)
{
    Canvas::VERTEX FanTri[] =
    {
        {{100, 100, 0, 1}, {255, 0, 0, 0}},
        {{80, 30, 0, 1}, {255, 0, 0, 0}},
        {{140, 30, 0, 1}, {255, 0, 0, 0}},
        {{140, 60, 0, 1}, {255, 0, 0, 0}},
    };
    pCanvas->DrawTriangleFan(FanTri, 4);
}

void Application::Update()
{
    auto pSwapChain = m_DXGIManager.GetSwapChain();
    auto pCanvas = m_DXGIManager.GetCanvas();
    static float hour = 8.0f;
    static float min = 20.0f;
    static float time = hour * 3600 + min * 60;
    float phi = time * 3.141592;
    float theta = 2 * 3.141592 * time;
    pCanvas->Clear({0, 0, 0, 0});

    Canvas* pTexture;
    pTexture = Canvas::CreateCanvas(300, 300);
    pTexture->Shade(ShaderNoise);
    pTexture->SetColorBorder({127, 127, 0, 0});
    pTexture->SetAddressMode(Canvas::ADDRESS_MODE_MIRROR);

    // Moviendo la textura al centro del canvas, utilizando transformaciones
    VECTOR4D Size = {(float)pTexture->GetSizeX(), (float)pTexture->GetSizeY(), 0.0f, 0.0f};
    VECTOR4D HalfSize = Size * 0.5f;
    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};
    VECTOR4D HalfScreenSize = ScreenSize * 0.5f;

    MATRIX4D M = Translation(-HalfSize.x, -HalfSize.y, 0.0f) * RotationZ(phi) * Translation(
        HalfScreenSize.x, HalfScreenSize.y, 0.0f);
    // Para aplicar el mapeo inverso, requerimos de invertir la matriz de transformación, a fin de poder aplicarlo a la textura
    // y no al canvas objetivo.
    MATRIX4D MInv;
    Inverse(M, MInv);

    for (int j = 0; j < pCanvas->GetSizeY(); j++)
    {
        for (int i = 0; i < pCanvas->GetSizeX(); i++)
        {
            VECTOR4D Input = {(float)i, (float)j, 0.0f, 1.0f};
            VECTOR4D Output = Input * MInv;
            (*pCanvas)(i, j) = pTexture->Peek((int)floorf(Output.x), (int)floorf(Output.y));
        }
    }

    if (m_pImage)
        for (int j = 0; j < m_pImage->GetSizeY(); j++)
            for (int i = 0; i < m_pImage->GetSizeX(); i++)
                (*pCanvas)(i, j) = (*m_pImage)(i, j);

    /*VECTOR4D O = {400, 400, 0, 1};
    
    VECTOR4D P1 = {50, 100, 0, 1}; //posiciones en pantalla con la componente homogénea en 1
    VECTOR4D V1 = {5, 6, 0, 0}; //vector con la componente homogénea en 0
    VECTOR4D V2 = {100, 100, 0, 0};

    VECTOR4D VR = V1 + V2;
    VECTOR4D P1P = P1 + V1;
    //OPERACIÓN NO VÁLIDA, ES SUMAR UN PUNTO A OTRO PUNTO, GENERA UNA COMPONENTE HOMOGÉNEA INVÁLIDA.

    VECTOR4D P2 = O + V2;

    MATRIX4D T0 = Translation(100, 100, 0);
    MATRIX4D R0 = RotationZ(theta);
    MATRIX4D R1 = RotationZ(theta / 60);
    MATRIX4D R2 = RotationZ(theta / 3600);
    VECTOR4D P1T = P1P * T0;
    VECTOR4D V2T = V2 * T0;
    VECTOR4D V2R = V2 * R0;
    VECTOR4D P2R = P2 - O;
    VECTOR4D P2R1 = P2 - O;
    VECTOR4D P2R2 = P2 - O;

    P2R = P2R * R0;
    P2R1 = P2R1 * R1;
    P2R = P2R + O;
    P2R1 = P2R1 + O;
    P2R2 = P2R2 * R2;
    P2R2 = P2R2 + O;
    
    pCanvas->Line(O.x, O.y, P2.x, P2.y, {255,255,255,0});
    pCanvas->Line(O.x, O.y, P2R.x, P2R.y, {255,0,0,0});
    pCanvas->Line(O.x, O.y, P2R1.x, P2R1.y, {0,255,0,0});
    pCanvas->Line(O.x, O.y, P2R2.x, P2R2.y, {0,0,255,0});
    */

    /*pCanvas->CircleLimits(200, 200, 100);
      pCanvas->FillLimits(ShaderChess);
      pCanvas->ResetLimits();
      pCanvas->CircleLimits(400, 200, 100);
      pCanvas->FillLimits(ShaderColors);
      */

    /*
    Canvas::VERTEX tri[] =
    {
        {{0,1,0,1}},
        {{1,-1,0,1}},
        {{-1,-1,0,1}}
    };

    Canvas::VERTEX OutputTri[3];

    MATRIX4D ST = Scaling(200, 200, 1) * RotationZ(theta) * Translation(600, 400, 0);
    Canvas::VertexProcessor(&ST, (Canvas::VERTEXSHADER)VertexShaderSimple, tri, OutputTri, 3);
    
    
    pCanvas->ResetLimits();

    pCanvas->DrawTriangleList(OutputTri, 3, {255, 255, 255, 0});
    */
    /*
    pCanvas->ResetLimits();
    
    ListPoints(pCanvas);
    ListLine(pCanvas, time);
    StripLines(pCanvas);
    FanTriangles(pCanvas);
    */
    //StripTriangles(pCanvas);

    // pCanvas->ResetLimits();

    m_DXGIManager.SendData(pCanvas->GetBuffer(),
                           pCanvas->GetPitch());

    // Almacenamos el último frame
    if (m_pLastFrame) Canvas::DestroyCanvas(m_pLastFrame);
    m_pLastFrame = pCanvas->Clone();

    if (pTexture) Canvas::DestroyCanvas(pTexture);
    Canvas::DestroyCanvas(pCanvas);
    pSwapChain->Present(1, 0);
    time += 1.0f / 60;
}

void Application::KeyEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'S':
            if (m_pLastFrame)
                m_pLastFrame->SaveCanvasToFile("..\\Data\\Save.bmp");
            break;
        case 'L':
            if (m_pImage) Canvas::DestroyCanvas(m_pImage);
            m_pImage = Canvas::CreateCanvasFromFile("..\\Data\\teamwork8bpp.bmp", nullptr);
        }
        break;
    }
}
