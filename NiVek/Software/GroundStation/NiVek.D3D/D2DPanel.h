#pragma once
#pragma once
#include "pch.h"
#include "DirectXPanelBase.h"
#include "StepTimer.h"
#include "ShaderStructures.h"
#include "DirectXPanelBase.h"


namespace NiVek
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref  class D2DPanel sealed : public NiVek::DirectXPanelBase
	{
	public:
		D2DPanel();

	private protected:
		virtual void Render() override;
		virtual void CreateDeviceResources() override;

		virtual void OnDeviceLost() override;
		virtual void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e) override;
		virtual void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^sender, Platform::Object ^args) override;
		virtual void OnResuming(Platform::Object^ sender, Platform::Object^ args) override;

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>                        m_strokeBrush;
	};
}
