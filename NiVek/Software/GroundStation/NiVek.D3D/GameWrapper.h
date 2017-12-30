#pragma once

#include "Game.h"

namespace NiVek
{
	public ref class GameWrapper sealed
	{
	public:
		GameWrapper(void);

		void Initialize(Windows::UI::Core::CoreWindow^ window, Windows::UI::Xaml::Controls::SwapChainBackgroundPanel^ panel, float dpi);
		void Update(float total, float delta);
		void Render();
		void Present();

	private:
		~GameWrapper();
	
		Game^ m_Game;

	};
}

