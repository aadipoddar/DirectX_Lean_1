#include "DXApp.h"

namespace
{
	//used to forward messages to user defined proc function
	DXApp * g_pApp = nullptr;
}

LRESULT CALLBACK MainWndProc ( HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	if ( g_pApp ) return g_pApp->MsgProc ( hwnd , msg , wParam , lParam );
	else return DefWindowProc ( hwnd , msg , wParam , lParam );
}

DXApp::DXApp ( HINSTANCE hInstance )
{
	m_hAppInstance		=	hInstance;
	m_hAppWind			=	NULL;
	m_ClientWidth		=	800;
	m_ClientHeight		=	600;
	m_AppTitle			=	"DIRECTX11 APPLICATION";
	m_WndStyle			=	WS_OVERLAPPEDWINDOW;
	g_pApp				=	this;

	m_pDevice			=	nullptr;
	m_pImmediateContext =	nullptr;
	m_pSwapChain		=	nullptr;
}

DXApp::~DXApp ( void )
{
	//Clean Up DIRECTX3D
	if ( m_pImmediateContext )m_pImmediateContext->ClearState ( );
	Memory::SafeRelease ( m_pRenderTargetView );
	Memory::SafeRelease ( m_pSwapChain );
	Memory::SafeRelease ( m_pImmediateContext );
	Memory::SafeRelease ( m_pDevice );
}

int DXApp::Run ( )
{
	//Main message log
	MSG msg = { 0 };
	while ( WM_QUIT != msg.message )
	{
		if ( PeekMessage ( &msg , NULL , NULL , NULL , PM_REMOVE ) )
		{
			TranslateMessage ( &msg );
			DispatchMessage ( &msg );
		}
		else
		{
			//update
			Update ( 0.0f );

			//render
			Render ( 0.0f );
		}
	}
	return static_cast< int >( msg.wParam );
}

bool DXApp::Init ( )
{
	if ( !InitWindow ( ) )
		return false;

	if ( !InitDirect3D ( ) )
	{
		return false;
	}

	return true;
}

bool DXApp::InitWindow ( )
{
	// Register the window class.
	const wchar_t CLASS_NAME [ ] = L"DXAPPWNDCLASS";

	//WNDCLASSEX
	WNDCLASSEX wcex;
	ZeroMemory ( &wcex , sizeof ( WNDCLASSEX ) );

	wcex.cbClsExtra			=	0;
	wcex.cbWndExtra			=	0;
	wcex.cbSize				=	sizeof ( WNDCLASSEX );
	wcex.style				=	CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance			=	m_hAppInstance;
	wcex.lpfnWndProc		=	MainWndProc;
	wcex.hIcon				=	LoadIcon ( NULL , IDI_APPLICATION );
	wcex.hCursor			=	LoadCursor ( NULL , IDC_ARROW );
	wcex.hbrBackground		=	( HBRUSH ) GetStockObject ( NULL_BRUSH );
	wcex.lpszMenuName		=	NULL;
	wcex.lpszClassName		=	CLASS_NAME;
	wcex.hIconSm			=	LoadIcon ( NULL , IDI_APPLICATION );

	if ( !RegisterClassEx ( &wcex ) )
	{
		OutputDebugStringA ( "\nFAILED TO CREATE WINDOW CLASS\n" );
		return false;
	}

	RECT r = { 0,0,m_ClientWidth,m_ClientHeight };
	AdjustWindowRect ( &r , m_WndStyle , FALSE );
	UINT width = r.right - r.left;
	UINT height = r.bottom - r.top;

	UINT x = GetSystemMetrics ( SM_CXSCREEN ) / 2 - width / 2;
	UINT y = GetSystemMetrics ( SM_CXSCREEN ) / 2 - height / 2;

	m_hAppWind = CreateWindowA ( "DXAPPWNDCLASS" , m_AppTitle.c_str ( ) , m_WndStyle , x , y , width , height , NULL , NULL , m_hAppInstance , NULL );

	if ( !m_hAppWind )
	{
		OutputDebugStringA ( "\nFAILED TO CREATE WINDOW\n" );
		return false;
	}

	ShowWindow ( m_hAppWind , SW_SHOW );

	return true;
}

bool DXApp::InitDirect3D ( )
{
	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	D3D_DRIVER_TYPE driverTypes [ ] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT	numDriverTypes = ARRAYSIZE ( driverTypes );

	D3D_FEATURE_LEVEL featureLevels [ ] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3
	};
	UINT numFeatureLevels = ARRAYSIZE ( featureLevels );

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory ( &swapDesc , sizeof ( DXGI_SWAP_CHAIN_DESC ) );

	swapDesc.BufferCount							=	1;//double buffered
	swapDesc.BufferDesc.Width						=	m_ClientWidth;
	swapDesc.BufferDesc.Height						=	m_ClientHeight;
	swapDesc.BufferDesc.Format						=	DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator		=	60;
	swapDesc.BufferDesc.RefreshRate.Denominator		=	1;
	swapDesc.BufferUsage							=	DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow							=	m_hAppWind;
	swapDesc.SwapEffect								=	DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed								=	true;
	swapDesc.SampleDesc.Count						=	1;
	swapDesc.SampleDesc.Quality						=	0;
	swapDesc.Flags									=	DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//alt-enter fullscreen

	HRESULT result;
	for ( int i = 0; i < numDriverTypes; ++i )
	{
		result = D3D11CreateDeviceAndSwapChain ( NULL , driverTypes [ i ] , NULL , createDeviceFlags , featureLevels , numFeatureLevels , D3D11_SDK_VERSION , &swapDesc , &m_pSwapChain , &m_pDevice , &m_FeatureLevel , &m_pImmediateContext );
		if ( SUCCEEDED ( result ) )
		{
			m_DriverType = driverTypes [ i ];
			break;
		}
	}

	if ( FAILED ( result ) )
	{
		OutputDebugStringA ( "FAILED TO CREATE DEVICE AND SWAP CHAIN" );
		return false;
	}

	//CREATE RENDER TARGET VIEW
	ID3D11Texture2D * m_pBackBufferTex = 0;
	HR ( m_pSwapChain->GetBuffer ( NULL , __uuidof( ID3D11Texture2D ) , reinterpret_cast< void ** >( &m_pBackBufferTex ) ) );
	HR ( m_pDevice->CreateRenderTargetView ( m_pBackBufferTex , nullptr , &m_pRenderTargetView ) );
	Memory::SafeRelease ( m_pBackBufferTex );

	//BIND RENDER TARGET VIEW
	m_pImmediateContext->OMSetRenderTargets ( 1 , &m_pRenderTargetView , nullptr );

	//VIEWPORT CREATION
	m_Viewport.Width		=	static_cast< float >( m_ClientWidth );
	m_Viewport.Height		=	static_cast< float >( m_ClientHeight );
	m_Viewport.TopLeftX		=	0;
	m_Viewport.TopLeftY		=	0;
	m_Viewport.MinDepth		=	0.0f;
	m_Viewport.MaxDepth		=	1.0f;

	//BIND VIEWPORT
	m_pImmediateContext->RSSetViewports ( 1 , &m_Viewport );

	return true;
}

LRESULT DXApp::MsgProc ( HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	switch ( msg )
	{
	case WM_DESTROY:
		PostQuitMessage ( 0 );
		return 0;

	default:
		return DefWindowProc ( hwnd , msg , wParam , lParam );
	}
}

