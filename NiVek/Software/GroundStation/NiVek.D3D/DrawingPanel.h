#pragma once

#include "pch.h"
#include "DirectXPanelBase.h"
#include "StepTimer.h"

#define MAX_CHANNELS 12

namespace NiVek
{
	// Event args for the RecognitionResultUpdated event.
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class RecognitionResultUpdatedEventArgs : public Windows::UI::Xaml::RoutedEventArgs
	{
	public:
		// Handwriting recognition results.
		property Platform::Array<Platform::String^>^ Results
		{
			Platform::Array<Platform::String^>^ get() { return m_results; }
		}

	internal:
		RecognitionResultUpdatedEventArgs(Platform::Array<Platform::String^>^ results)
		{
			m_results = results;
		}

	private protected:
		Platform::Array<Platform::String^>^ m_results;
	};

	// Delegate for the RecognitionResultUpdated event.
	[Windows::Foundation::Metadata::WebHostHidden]
	public delegate void RecognitionResultUpdatedEventHandler(Platform::Object^ sender, NiVek::RecognitionResultUpdatedEventArgs^ s);

	// Hosts a DirectX rendering surface that supports various inking and drawing operations, in addition to raising handwriting recognition events.

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DrawingPanel sealed : public NiVek::DirectXPanelBase
	{
	public:
		DrawingPanel();

#pragma region DependencyProperties        
		static void RegisterDependencyProperties();

		static property Windows::UI::Xaml::DependencyProperty^ BrushColorProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return m_brushColorProperty; }
		}
		property Windows::UI::Color BrushColor
		{
			// Additionally keep a local value for thread-safe property access.
			Windows::UI::Color get() { return m_brushColor; }
			void set(Windows::UI::Color value)
			{
				m_brushColor = value;
				SetValue(BrushColorProperty, value);
			}
		}

		static property Windows::UI::Xaml::DependencyProperty^ BrushFitsToCurveProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return m_brushFitsToCurveProperty; }
		}
		property bool BrushFitsToCurve
		{
			bool get() { return safe_cast<bool>(GetValue(BrushFitsToCurveProperty)); }
			void set(bool value) { SetValue(BrushFitsToCurveProperty, value); }
		}

		static property Windows::UI::Xaml::DependencyProperty^ BrushSizeProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return m_brushSizeProperty; }
		}
		property Windows::Foundation::Size BrushSize
		{
			// Additionally keep a local value for thread-safe property access.
			Windows::Foundation::Size get() { return m_brushSize; }
			void set(Windows::Foundation::Size value)
			{
				m_brushSize = value;
				SetValue(BrushSizeProperty, value);
			}
		}		

		static property Windows::UI::Xaml::DependencyProperty^ BrushIsEraserProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return m_brushIsEraserProperty; }
		}

		property bool BrushIsEraser
		{
			// Keep a local value for thread-safe property reads
			bool get() { return m_brushIsEraser; }
			void set(bool value)
			{
				m_brushIsEraser = value;
				SetValue(BrushIsEraserProperty, value);
			}
		}
#pragma endregion 

		void Update();

		void SetChannelVisibility(int channelIndex, bool visibility);
		void SetChannelColor(int channelIndex, Windows::UI::Color color);
		void SetNumberChannels(int numberChannels);
		void LoadFromStream(int channelIndex, Windows::Storage::Streams::IRandomAccessStream^ stream, int intervalInMilliseconds);
		void StartRenderLoop();
		void StopRenderLoop();
		void StartProcessingInput();
		void StopProcessingInput();

	private protected:
		enum DrawingState {
			Uninitialized = 0,
			None,
			Ready,
		};

		virtual void Render() override;
		virtual void CreateDeviceResources() override;
		virtual void CreateSizeDependentResources() override;

		virtual void OnDeviceLost() override;
		virtual void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e) override;
		virtual void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^sender, Platform::Object ^args) override;
		virtual void OnResuming(Platform::Object^ sender, Platform::Object^ args) override;

		// DependencyProperties
		static void BrushColorValueChanged(Windows::UI::Xaml::DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void BrushFitsToCurveValueChanged(Windows::UI::Xaml::DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void BrushSizeValueChanged(Windows::UI::Xaml::DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void BrushIsEraserValueChanged(Windows::UI::Xaml::DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);

		Windows::UI::Core::CoreIndependentInputSource^						m_coreInput;
		static Windows::UI::Xaml::DependencyProperty^                       m_brushColorProperty;
		static Windows::UI::Xaml::DependencyProperty^                       m_brushFitsToCurveProperty;
		static Windows::UI::Xaml::DependencyProperty^                       m_brushSizeProperty;
		Windows::Foundation::Size											m_brushSize;
		static Windows::UI::Xaml::DependencyProperty^                       m_brushIsEraserProperty;
		bool																m_brushIsEraser;
		bool                                                                m_brushFitsToCurve;
		Windows::UI::Color                                                  m_brushColor;

		DrawingState                                                        m_drawingState;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>                             m_currentBuffer;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>                             m_previousBuffer;

		Microsoft::WRL::ComPtr<ID2D1StrokeStyle>                            m_inkStrokeStyle;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>                        m_strokeBrush;

		Windows::Foundation::IAsyncAction^					m_renderLoopWorker;
		Windows::Foundation::IAsyncAction^									m_inputLoopWorker;

		Microsoft::WRL::ComPtr<IDXGIOutput>                 m_dxgiOutput;
		DX::StepTimer                                       m_timer;

		ID2D1PathGeometry									*m_pChartLineGeometry;
		
		D2D1_POINT_2F							m_ChartPoints[4096][MAX_CHANNELS];
		int										m_NumberPoints[MAX_CHANNELS];
		int										m_NumberActiveChannels;

		Windows::UI::Color						m_brushColors[MAX_CHANNELS];
		bool									m_channelVisibility[MAX_CHANNELS];
	};
}
