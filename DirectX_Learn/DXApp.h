#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include "DXUtil.h"

class DXApp
{
public:
	DXApp ( HINSTANCE hInstance );
	virtual ~DXApp ( void );

	//main application loop
	int Run ( );

	//framework methods
	virtual bool Init ( );
	virtual void Update ( float dt ) = 0;
	virtual void Render ( float dt ) = 0;
	virtual LRESULT MsgProc ( HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam );

protected:

	//win32 attributes
	HWND			m_hAppWind;
	HINSTANCE		m_hAppInstance;
	UINT			m_ClientWidth;
	UINT			m_ClientHeight;
	std::string		m_AppTitle;
	DWORD			m_WndStyle;

	//DIRECTX attributes
	ID3D11Device*				m_pDevice;
	ID3D11DeviceContext*		m_pImmediateContext;
	IDXGISwapChain*				m_pSwapChain;
	ID3D11RenderTargetView*		m_pRenderTargetView;
	D3D_DRIVER_TYPE				m_DriverType;
	D3D_FEATURE_LEVEL			m_FeatureLevel;
	D3D11_VIEWPORT				m_Viewport;

protected:

	//INITIALIZE win32 window
	bool InitWindow ( );

	//INITIALIZE DIRECT3D
	bool InitDirect3D ( );
};

