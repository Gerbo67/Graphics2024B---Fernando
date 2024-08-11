#pragma once
#include "framework.h"
#include "DXGIManager.h"
class Application
{
public:
	// Add application data here!
	HWND m_hWnd;
	HINSTANCE hInstance;
	int  m_nCmdShow;
	DXGIManager m_DXGIManager;
	IDXGIAdapter* m_pAdapter;
	void Run();
	static Application* GetApplication();
	Canvas* m_pLastFrame;
	void Event(UINT msg, WPARAM wParam, LPARAM lParam);
	Canvas* m_pImage;
	Canvas* m_pSun;
	Canvas* m_pMercury;
	Canvas* m_pVenus;
	Canvas* m_pEarth;
	Canvas* m_pMars;
	Canvas* m_pJupiter;
	Canvas* m_pSaturn;
	Canvas* m_pUranus;
	Canvas* m_pNeptune;
	Canvas* m_pPluto;
	int m_nMouseX;
	int m_nMouseY;
protected:
	static Application* sm_pApp;
	Application();
	~Application();
	bool Initialize();
	bool Uninitialize();
	void Update();
};

