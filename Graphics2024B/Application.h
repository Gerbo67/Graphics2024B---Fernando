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
	void KeyEvent(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	static Application* sm_pApp;
	Application();
	~Application();
	bool Initialize();
	bool Uninitialize();
	void Update();
};

