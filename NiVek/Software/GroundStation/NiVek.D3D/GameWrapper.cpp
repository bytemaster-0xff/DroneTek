#include "pch.h"
#include "GameWrapper.h"


namespace NiVek
{
	GameWrapper::GameWrapper(void)
	{
		m_Game = ref new Game();
	}

	void GameWrapper::Initialize(Windows::UI::Core::CoreWindow^ window, Windows::UI::Xaml::Controls::SwapChainBackgroundPanel^ panel, float dpi)
	{
		m_Game->Initialize(window, panel, dpi);
	}

	void GameWrapper::Update(float total, float delta){
		m_Game->Update(total, delta);
	}

	void GameWrapper::Render(){
		m_Game->Render();
	}
	void GameWrapper::Present(){
		m_Game->Present();
	}


	GameWrapper::~GameWrapper(void)
	{
	}
}
