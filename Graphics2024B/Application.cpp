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
    m_pImage = nullptr;

    m_pSun = nullptr;
    m_pMercury = nullptr;
    m_pVenus = nullptr;
    m_pEarth = nullptr;
    m_pMars = nullptr;
    m_pJupiter = nullptr;
    m_pSaturn = nullptr;
    m_pUranus = nullptr;
    m_pNeptune = nullptr;
    m_pPluto = nullptr;
    m_nMouseX = m_nMouseY = 0;
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
    // Cargando texturas
    m_pSun = Canvas::CreateCanvasFromFile("..\\Data\\sol.bmp", nullptr);
    m_pMercury = Canvas::CreateCanvasFromFile("..\\Data\\mercurio.bmp", nullptr);
    m_pVenus = Canvas::CreateCanvasFromFile("..\\Data\\venus.bmp", nullptr);
    m_pEarth = Canvas::CreateCanvasFromFile("..\\Data\\tierra.bmp", nullptr);
    m_pMars = Canvas::CreateCanvasFromFile("..\\Data\\marte.bmp", nullptr);
    m_pJupiter = Canvas::CreateCanvasFromFile("..\\Data\\jupiter.bmp", nullptr);
    m_pSaturn = Canvas::CreateCanvasFromFile("..\\Data\\saturno.bmp", nullptr);
    m_pUranus = Canvas::CreateCanvasFromFile("..\\Data\\urano.bmp", nullptr);
    m_pNeptune = Canvas::CreateCanvasFromFile("..\\Data\\neptuno.bmp", nullptr);
    m_pPluto = Canvas::CreateCanvasFromFile("..\\Data\\pluton.bmp", nullptr);
    if (!m_pSun || !m_pMercury || !m_pEarth || !m_pMars || !m_pJupiter || !m_pSaturn || !m_pUranus || !m_pNeptune || !
        m_pPluto)
        return false;

    return true;
}

bool Application::Uninitialize()
{
    if (m_pLastFrame) Canvas::DestroyCanvas(m_pLastFrame);
    if (m_pImage) Canvas::DestroyCanvas(m_pLastFrame);
    if (m_pSun) Canvas::DestroyCanvas(m_pSun);
    if (m_pMercury) Canvas::DestroyCanvas(m_pMercury);
    if (m_pEarth) Canvas::DestroyCanvas(m_pEarth);
    if (m_pMars) Canvas::DestroyCanvas(m_pMars);
    if (m_pJupiter) Canvas::DestroyCanvas(m_pJupiter);
    if (m_pSaturn) Canvas::DestroyCanvas(m_pSaturn);
    if (m_pUranus) Canvas::DestroyCanvas(m_pUranus);
    if (m_pNeptune) Canvas::DestroyCanvas(m_pNeptune);
    if (m_pPluto) Canvas::DestroyCanvas(m_pPluto);
    return true;
}

void VertexShaderSimple(MATRIX4D* pM, Canvas::VERTEX& Input, Canvas::VERTEX& Output)
{
    Output.P = Input.P * (*pM);
    Output.TexCoord = Input.TexCoord;
}

void RotateAndDrawQuads(Canvas* pCanvas, int distance, float phi, Canvas::VERTEX Squad[], Canvas* textures[],
                        int numTextures, int currentTextureIndex)
{
    Canvas::PIXEL color = Canvas::PIXEL({0, 0, 0, 0});
    // Obtén las dimensiones de la pantalla
    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};

    // Gracias a la función "RotateAndDrawMultipleQuads", ahora podemos controlar la distancia desde la "distance"
    VECTOR4D translation = {(float)distance, 0, 0, 0};

    // Crea una matriz de transformación:
    // 1. Traslada el cuadrado al centro de la pantalla.
    // 2. Aplica la rotación.
    // 3. Traslada el cuadrado con la distancia hacia la ubicación final del cuadrado.
    // Cambio realizado: ajustar la secuencia de transformación para rotar correctamente los cuadrados alrededor del centro de la pantalla
    MATRIX4D M = Translation(translation.x, translation.y, 0) *
        RotationZ(phi / distance) *
        Translation(ScreenSize.x / 2, ScreenSize.y / 2, 0);

    // Procesa los vértices con la matriz de transformación
    Canvas::VERTEX Transformed[4];
    Canvas::VertexProcessor(&M, (Canvas::VERTEXSHADER)VertexShaderSimple, Squad, Transformed, 4);

    // Dibuja el cuadrado utilizando un listado de triángulos (dos triángulos que forman el cuadrado)
    pCanvas->DrawQuad(Transformed, color);
    pCanvas->TextureMappingQuad(Transformed, textures[currentTextureIndex % numTextures]);
}

void DrawMultipleCircles(Canvas* pCanvas, int xc, int yc, int initialRadius, int nCircles, int radiusIncrement,
                         Canvas::PIXEL color, float phi, Canvas* textures[], int numTextures)
{
    Canvas::VERTEX Squad[] = {
        {{-20, -20, 0, 1}, color, {0, 0, 0, 1}},
        {{20, -20, 0, 1}, color, {1, 0, 0, 1}},
        {{20, 20, 0, 1}, color, {1, 1, 0, 1}},
        {{-20, 20, 0, 1}, color, {0, 1, 0, 1}},
    };

    for (int i = 0; i < nCircles; ++i)
    {
        int newRadius = initialRadius + (i * radiusIncrement);
        pCanvas->Circle(xc, yc, newRadius, color);
        // gira los cuadrados en cada círculo
        RotateAndDrawQuads(pCanvas, newRadius, ((2 * phi) / (newRadius + 1)), Squad, textures, numTextures, i);
    }

    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};

    MATRIX4D M = Translation(ScreenSize.x / 2, ScreenSize.y / 2, 0);
    Canvas::VERTEX Transformed[4];
    Canvas::VertexProcessor(&M, (Canvas::VERTEXSHADER)VertexShaderSimple, Squad, Transformed, 4);

    // Dibuja el cuadrado utilizando un listado de triángulos (dos triángulos que forman el cuadrado)
    pCanvas->DrawQuad(Transformed, Canvas::PIXEL({0, 0, 0, 0}));
    pCanvas->TextureMappingQuad(Transformed, textures[9]);
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
    float scale = 1000.0;
    float phi = scale * time * 3.141592;
    float theta = 2 * 3.141592 * time;
    pCanvas->Clear({0, 0, 0, 0});

    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};
    Canvas* planetTextures[] = {
        m_pMercury, m_pVenus, m_pEarth, m_pMars, m_pJupiter, m_pSaturn, m_pUranus, m_pNeptune, m_pPluto, m_pSun
    };
    DrawMultipleCircles(pCanvas, ScreenSize.x / 2, ScreenSize.y / 2, 60, 9, 45, Canvas::PIXEL({255, 255, 255, 0}), phi,
                        planetTextures, 10);


    m_DXGIManager.SendData(pCanvas->GetBuffer(),
                           pCanvas->GetPitch());


    // Almacenamos el último frame
    if (m_pLastFrame) Canvas::DestroyCanvas(m_pLastFrame);
    m_pLastFrame = pCanvas->Clone();

    //if (pTexture) Canvas::DestroyCanvas(pTexture);

    Canvas::DestroyCanvas(pCanvas);
    pSwapChain->Present(1, 0);
    time += 1.0f / 60;
}

void Application::Event(UINT msg, WPARAM wParam, LPARAM lParam)
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
            m_pImage = Canvas::CreateCanvasFromFile("..\\Data\\pluton.bmp", nullptr);
        }
        break;
    case WM_MOUSEMOVE:
        m_nMouseX = LOWORD(lParam);
        m_nMouseY = HIWORD(lParam);
        break;
    }
}
