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

    m_pUniverse = nullptr;
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
    m_pUniverse = Canvas::CreateCanvasFromFile("..\\Data\\universe.bmp", nullptr);
    m_pSun = Canvas::CreateCanvasFromFile("..\\Data\\sun.bmp", nullptr);
    m_pMercury = Canvas::CreateCanvasFromFile("..\\Data\\mercury.bmp", nullptr);
    m_pVenus = Canvas::CreateCanvasFromFile("..\\Data\\venus.bmp", nullptr);
    m_pEarth = Canvas::CreateCanvasFromFile("..\\Data\\earth.bmp", nullptr);
    m_pMars = Canvas::CreateCanvasFromFile("..\\Data\\mars.bmp", nullptr);
    m_pJupiter = Canvas::CreateCanvasFromFile("..\\Data\\jupiter.bmp", nullptr);
    m_pSaturn = Canvas::CreateCanvasFromFile("..\\Data\\saturn.bmp", nullptr);
    m_pUranus = Canvas::CreateCanvasFromFile("..\\Data\\uranus.bmp", nullptr);
    m_pNeptune = Canvas::CreateCanvasFromFile("..\\Data\\neptune.bmp", nullptr);
    m_pPluto = Canvas::CreateCanvasFromFile("..\\Data\\pluto.bmp", nullptr);
    if (!m_pSun || !m_pMercury || !m_pEarth || !m_pMars || !m_pJupiter || !m_pSaturn || !m_pUranus || !m_pNeptune || !
        m_pPluto || !m_pUniverse)
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

// Aplica las transformaciones necesarias a los vértices
void ApplyVertexTransforms(Canvas* pCanvas, float distance, float phi, Canvas::VERTEX Squad[],
                           Canvas::VERTEX Transformed[])
{
    // Define color del pixel
    Canvas::PIXEL color = Canvas::PIXEL({0, 0, 0, 0});
    // Obtén las dimensiones de la screen
    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};
    // Define la traslación basada en la distancia
    VECTOR4D translation = {(float)distance, 0, 0, 0};
    // Crea una matriz de transformación
    MATRIX4D M = Translation(translation.x, translation.y, 0) *
        RotationZ(phi / distance) *
        Translation(ScreenSize.x / 2, ScreenSize.y / 2, 0);
    // Procesa los vértices con la matriz de transformación
    Canvas::VertexProcessor(&M, (Canvas::VERTEXSHADER)VertexShaderSimple, Squad, Transformed, 4);
}

// Dibuja un cuadrado y aplica mapeo de textura
void DrawAndApplyTextureMapping(Canvas* pCanvas, Canvas::VERTEX Transformed[], Canvas::PIXEL color, Canvas* texture)
{
    // Dibuja el cuadrado
    pCanvas->DrawQuad(Transformed, color);
    // Aplica mapeo de textura
    pCanvas->TextureMappingQuad(Transformed, texture);
}

// Metodo intermedio para aplicar rotacion, dibujar y texturizar
void RotateDrawQuads(Canvas* pCanvas, int distance, float phi, Canvas::VERTEX Squad[], Canvas* planetTextures[],
                     int numTextures, int currentTextureIndex)
{
    // Cuadrados negros por tema de estetica
    Canvas::PIXEL color = {0, 0, 0, 0};
    Canvas::VERTEX Transformed[4];

    // Aplica transformaciones a los vértices
    ApplyVertexTransforms(pCanvas, (float)distance, phi, Squad, Transformed);
    // Dibuja el cuadrado y aplica mapeo de textura
    DrawAndApplyTextureMapping(pCanvas, Transformed, color, planetTextures[currentTextureIndex % numTextures]);
}



void DrawCirclesSquads(Canvas* pCanvas, float phi, Canvas* planetTextures[])
{
    int firstRadius = 80;
    int moreCirclesDistance = 45;
    int numCircles = 9;

    // Color circulos en blanco
    Canvas::PIXEL color = {255, 255, 255, 0};

    Canvas::VERTEX Transformed[4];

    // Definir el tamaño de los planetas
    int planetSize [] ={10, 15, 20, 17, 40, 30, 20, 20, 5, 50};


    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};

    for (int i = 0; i < numCircles; ++i)
    {
        float size = planetSize[i]; 
        Canvas::VERTEX Squad[] = {
            {{-size, -size, 0, 1}, color, {0, 0, 0, 1}},
            {{size, -size, 0, 1}, color, {1, 0, 0, 1}},
            {{size, size, 0, 1}, color, {1, 1, 0, 1}},
            {{-size, size, 0, 1}, color, {0, 1, 0, 1}},
        };
        int newRadius = firstRadius + (i * moreCirclesDistance);
        // Dibuja el círculo
        pCanvas->Circle((int)ScreenSize.x / 2, (int)ScreenSize.y / 2, newRadius, color);
        // Rota y dibuja los cuadrados
        RotateDrawQuads(pCanvas, newRadius, ((2 * phi) / (newRadius + 1)), Squad, planetTextures, numCircles, i);
    }

    float size = planetSize[9]; 
    Canvas::VERTEX Squad[] = {
        {{-size, -size, 0, 1}, color, {0, 0, 0, 1}},
        {{size, -size, 0, 1}, color, {1, 0, 0, 1}},
        {{size, size, 0, 1}, color, {1, 1, 0, 1}},
        {{-size, size, 0, 1}, color, {0, 1, 0, 1}},
    };

    // Mover al centro para mostrar el sol en medio
    MATRIX4D M = Translation(ScreenSize.x / 2, ScreenSize.y / 2, 0);
    Canvas::VertexProcessor(&M, (Canvas::VERTEXSHADER)VertexShaderSimple, Squad, Transformed, 4);

    // Dibuja el cuadrado y texturizar el sol
    pCanvas->DrawQuad(Transformed, Canvas::PIXEL({0, 0, 0, 0}));
    pCanvas->TextureMappingQuad(Transformed, planetTextures[9]);
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
    /*
     * José Fernando Resendiz Lopez - 38081
     */
    auto pSwapChain = m_DXGIManager.GetSwapChain();
    auto pCanvas = m_DXGIManager.GetCanvas();
    static float hour = 8.0f;
    static float min = 20.0f;
    static float time = hour * 3600 + min * 60;
    float phi = time * 3.141592 * (1500);
    float theta = time * 3.141592;
    pCanvas->Clear({0, 0, 0, 0});

    Canvas* planetTextures[] = {
        m_pMercury, m_pVenus, m_pEarth, m_pMars, m_pJupiter, m_pSaturn, m_pUranus, m_pNeptune, m_pPluto, m_pSun
    };

    // Texturizar el espacio
    Canvas* pTexture = nullptr;
    pTexture = m_pUniverse->Clone();
    pTexture->SetColorBorder({127, 127, 0, 0});
    pTexture->SetAddressMode(Canvas::ADDRESS_MODE_MIRROR);
    // Moviendo la textura al centro del canvas, utilizando transformaciones
    VECTOR4D Size = {(float)pTexture->GetSizeX(), (float)pTexture->GetSizeY(), 0.0f, 0.0f};
    VECTOR4D HalfSize = Size * 0.5f;
    VECTOR4D ScreenSize = {(float)pCanvas->GetSizeX(), (float)pCanvas->GetSizeY(), 0.0f, 0.0f};
    VECTOR4D HalfScreenSize = ScreenSize * 0.5f;
    MATRIX4D M = Scaling((ScreenSize.x / Size.x) * 0.5f, (ScreenSize.y / Size.y) * 0.5f, 1) * RotationZ(theta / 5);
    // Para aplicar el mapeo inverso, requerimos de invertir la matriz de transformación, a fin de poder aplicarlo a la textura
    // y no al canvas objetivo.
    MATRIX4D MInv;
    Inverse(M, MInv);

    float p, q;

    for (int j = 0; j < pCanvas->GetSizeY(); j++)
    {
        p = j * MInv.m10 + MInv.m30;
        q = j * MInv.m11 + MInv.m31;

        for (int i = 0; i < pCanvas->GetSizeX(); i++)
        {
            //VECTOR4D Input = {(float)i, (float)j, 0.0f, 1.0f};
            //VECTOR4D Output = Input * MInv;
            //(*pCanvas)(i, j) = pTexture->Peek((int)floorf(Output.x), (int)floor(Output.y));
            //(*pCanvas)(i, j) = pTexture->Peek((int)floorf(p), (int)floor(q));
            (*pCanvas)(i, j) = pTexture->PointSampler(p, q);
            //(*pCanvas)(i, j) = pTexture->BilinearSampler(p, q);

            p += MInv.m00;
            q += MInv.m01;
        }
    }

    // Dibujar planetas
    DrawCirclesSquads(pCanvas, phi, planetTextures);

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
