#include "DXGIManager.h"
#include "Canvas.h"
/* DirectX se basa en tecnologia Microsoft COM 
   Component Object Model 
*/
IDXGIAdapter* DXGIManager::ChooseAdapter()
{
	IDXGIFactory* pFactory = NULL;
	IDXGIAdapter* pAdapter = NULL;
	CreateDXGIFactory(IID_IDXGIFactory, (void**)&pFactory);
	int iAdapter = 0;
	do
	{
		if(FAILED(pFactory->EnumAdapters(iAdapter, &pAdapter)))
			return nullptr;
		DXGI_ADAPTER_DESC dad;
		pAdapter->GetDesc(&dad);
		WCHAR szMessage[1024];
		wsprintf(szMessage, L"Descripción %s", dad.Description);
		switch (MessageBox(NULL, szMessage, L"¿Desea usar el siguiente adaptador?", MB_YESNOCANCEL))
		{
		case IDYES:
			pFactory->Release();
			return pAdapter;
		case IDNO:
			pAdapter->Release();
			iAdapter++;
			break;
		case IDCANCEL:
			pAdapter->Release();
			pFactory->Release();
			return nullptr;
		}
	} while (1);
	return nullptr;
}
#include "Application.h"
boolean DXGIManager::InitializeSwapChain(IDXGIAdapter* pAdapter)
{
	Application* pApp=Application::GetApplication();
	D3D_FEATURE_LEVEL requested = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL granted;
	DXGI_SWAP_CHAIN_DESC dscd;
	RECT rc;
	memset(&dscd, 0, sizeof(dscd));
	dscd.BufferCount = 2;
	GetClientRect(pApp->m_hWnd, &rc);
	dscd.BufferDesc.Width = rc.right;
	dscd.BufferDesc.Height = rc.bottom;
	dscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	/*dscd.BufferDesc.RefreshRate.Numerator = 5994;
	  dscd.BufferDesc.RefreshRate.Denominator = 100; */
	dscd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	dscd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	dscd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | /* D3D11 */
		               DXGI_USAGE_UNORDERED_ACCESS;      /* DC11  */
	dscd.OutputWindow = pApp->m_hWnd;
	dscd.SampleDesc.Count = 1;
	dscd.Windowed = true;
	dscd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr = 
	D3D11CreateDeviceAndSwapChain(
		pAdapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, &requested, 1, D3D11_SDK_VERSION,
		&dscd, &m_pSwapChain, &m_pDev, &granted, &m_pCtx);
	return SUCCEEDED(hr);
}

void DXGIManager::SendData(void* pBuffer, unsigned int pitch)
{
	ID3D11Texture2D* pBackbuffer=nullptr;
	ID3D11Texture2D* pStaging = nullptr;
	D3D11_TEXTURE2D_DESC dtd;
	m_pSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&pBackbuffer);
	pBackbuffer->GetDesc(&dtd);
	dtd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dtd.Usage = D3D11_USAGE_STAGING;
	dtd.BindFlags = 0;
	D3D11_SUBRESOURCE_DATA dsd;
	dsd.pSysMem = pBuffer;
	dsd.SysMemPitch = pitch;
	dsd.SysMemSlicePitch = 0;
	/* Transferir información desde el buffer de usuario al buffer de kernel */
	m_pDev->CreateTexture2D(&dtd, &dsd, &pStaging);
	/* Transferir información desde la memoria de kernel a dispositivo */
	if (pBackbuffer && pStaging)
		m_pCtx->CopyResource(pBackbuffer, pStaging);
	if (pStaging)
		pStaging->Release();
	if(pBackbuffer)
		pBackbuffer->Release();
}

Canvas* DXGIManager::GetCanvas()
{
	D3D11_TEXTURE2D_DESC dtd;
	ID3D11Texture2D* pBackbuffer = nullptr;
	m_pSwapChain->GetBuffer(0, IID_ID3D11Texture2D,
		(void**)&pBackbuffer);
	pBackbuffer->GetDesc(&dtd);
	Canvas* pCanvas = Canvas::CreateCanvas(
		dtd.Width, dtd.Height);
	pBackbuffer->Release();
	return pCanvas;
}


