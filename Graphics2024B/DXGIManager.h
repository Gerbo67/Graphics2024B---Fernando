#pragma once
#include "framework.h"
#include <dxgi.h>
#include <d3d11.h>

class Canvas;

class DXGIManager
{
protected:
	IDXGISwapChain*      m_pSwapChain;      // Cadena de intercambio
	ID3D11Device*        m_pDev;            // Abstracción de dispositivo
	ID3D11DeviceContext* m_pCtx;            // Operaciones sobre un dispositivo
public:
	IDXGIAdapter* ChooseAdapter();
	boolean InitializeSwapChain(IDXGIAdapter* pAdapter);
	IDXGISwapChain* GetSwapChain() { return m_pSwapChain; }
	void SendData(void* pBuffer,      /* Buffer */
		          unsigned int pitch); /* Horizontal Length*/
		          
	Canvas* GetCanvas();
};

