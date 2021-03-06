#include <memory>
#include "DXApp.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <DDSTextureLoader.h>
#include <SimpleMath.h>

class TestApp : public DXApp
{
public:
	TestApp ( HINSTANCE hInstance );
	~TestApp ( );

	bool Init ( ) override;
	void Update ( float dt )override;
	void Render ( float dt )override;

private:

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;
	ID3D11ShaderResourceView * m_pTexture;
};


TestApp::TestApp ( HINSTANCE hInstance ) : DXApp ( hInstance )
{
	m_AppTitle = "DirectX Learn";
}

TestApp::~TestApp ( )
{
	Memory::SafeRelease ( m_pTexture );
}

bool TestApp::Init ( )
{
	if ( !DXApp::Init ( ) )
		return false;

	//CREATE SPRITEBATCH OBJECTS
	spriteBatch.reset ( new DirectX::SpriteBatch ( m_pImmediateContext ) );

	//CREATE SPRITEFONT OBJECT
	spriteFont.reset ( new DirectX::SpriteFont ( m_pDevice , L"Arial.spriteFont" ) );

	//IMPORT TEXTURE FOR RENDERING
	HR ( DirectX::CreateDDSTextureFromFile ( m_pDevice , L"Test.dds" , nullptr , &m_pTexture ) );

	return true;
}

void TestApp::Update ( float dt )
{
}

void TestApp::Render ( float dt )
{
	m_pImmediateContext->ClearRenderTargetView ( m_pRenderTargetView , DirectX::Colors::Purple );

	spriteBatch->Begin ( );

	//DRAW Sprites, FONTS, ETC.
	spriteBatch->Draw ( m_pTexture , DirectX::SimpleMath::Vector2 ( 100 , 100 ) );

	//DRAW FONT
	spriteFont->DrawString ( spriteBatch.get ( ) , "HELLO WORLD" , DirectX::SimpleMath::Vector2 ( 300 , 300 ) );

	spriteBatch->End ( );

	HR(m_pSwapChain->Present ( 0 , 0 ));
}

int WINAPI WinMain ( __in HINSTANCE hInstance , __in_opt HINSTANCE hPrevInstance , __in LPSTR lpCmdLine , __in int nShowCmd )
{
	TestApp tApp ( hInstance );

	if ( !tApp.Init ( ) )return 1;

	return tApp.Run ( );
}