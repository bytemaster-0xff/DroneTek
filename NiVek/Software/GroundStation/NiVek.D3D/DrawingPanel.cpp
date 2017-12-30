#include "pch.h"
#include "DrawingPanel.h"
#include <DirectXMath.h>
#include <math.h>
#include <ppltasks.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include "DXHelper.h"

using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Input::Inking;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Interop;
using namespace Concurrency;
using namespace DirectX;
using namespace D2D1;
using namespace NiVek;
using namespace DX;

// Initialize dependency properties to null.  The app must call RegisterDependencyProperties() before using them.
DependencyProperty^ DrawingPanel::m_brushColorProperty = nullptr;
DependencyProperty^ DrawingPanel::m_brushFitsToCurveProperty = nullptr;
DependencyProperty^ DrawingPanel::m_brushSizeProperty = nullptr;
DependencyProperty^ DrawingPanel::m_brushIsEraserProperty = nullptr;

DrawingPanel::DrawingPanel() :
m_drawingState(DrawingState::Uninitialized),
m_pChartLineGeometry(NULL)
{
	critical_section::scoped_lock lock(m_criticalSection);

	CreateDeviceIndependentResources();
	CreateDeviceResources();
	CreateSizeDependentResources();

	m_NumberActiveChannels = 0;

	for (int idx = 0; idx < MAX_CHANNELS; ++idx)
	{
		m_channelVisibility[idx] = true;
		m_brushColors[idx] = Windows::UI::Colors::Black;
		m_NumberPoints[idx] = 0;
	}
}

// Static method to register custom DependencyProperties of the DrawingPanel class.  This should be called from the App object constructor of
// any app that uses this class to ensure the DependencyProperties are correctly registered before an instance of DrawingPanel is created.
void DrawingPanel::RegisterDependencyProperties()
{
	// Ensure properties are only registered once.
	if (m_brushColorProperty == nullptr)
	{
		m_brushColorProperty = DependencyProperty::Register("BrushColor", Color::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushColorValueChanged)));
	}

	if (m_brushFitsToCurveProperty == nullptr)
	{
		m_brushFitsToCurveProperty = DependencyProperty::Register("BrushFitsToCurve", bool::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushFitsToCurveValueChanged)));
	}

	if (m_brushSizeProperty == nullptr)
	{
		m_brushSizeProperty = DependencyProperty::Register("BrushSize", Windows::Foundation::Size::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushSizeValueChanged)));
	}

	if (m_brushIsEraserProperty == nullptr)
	{
		m_brushIsEraserProperty = DependencyProperty::Register("BrushIsEraser", bool::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushIsEraserValueChanged)));
	}
}



#pragma region DependencyProperty change handlers
void DrawingPanel::BrushColorValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
	auto panel = safe_cast<DrawingPanel^>(sender);
	auto color = safe_cast<Color>(e->NewValue);

	critical_section::scoped_lock lock(panel->m_criticalSection);
	panel->m_brushColor = color;
	panel->m_d2dContext->CreateSolidColorBrush(DX::ConvertToColorF(color), &panel->m_strokeBrush);
}

void DrawingPanel::BrushFitsToCurveValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
	auto panel = safe_cast<DrawingPanel^>(sender);
	auto value = safe_cast<bool>(e->NewValue);

	panel->m_brushFitsToCurve = value;
}

void DrawingPanel::BrushSizeValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
	auto panel = safe_cast<DrawingPanel^>(sender);
	auto value = safe_cast<Windows::Foundation::Size>(e->NewValue);

	panel->m_brushSize = value;
}

void DrawingPanel::BrushIsEraserValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
	auto panel = safe_cast<DrawingPanel^>(sender);
	auto value = safe_cast<bool>(e->NewValue);

	panel->m_brushIsEraser = value;
}
#pragma endregion

void DrawingPanel::StartProcessingInput()
{
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^)
	{
		m_coreInput = CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		m_drawingState = DrawingState::None;

		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
	
}

void DrawingPanel::StopProcessingInput()
{
	m_coreInput->Dispatcher->StopProcessEvents();
}

void DrawingPanel::CreateDeviceResources()
{
	DirectXPanelBase::CreateDeviceResources();

	ComPtr<IDXGIFactory1> dxgiFactory;
	ThrowIfFailed(
		CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))
		);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(
		dxgiFactory->EnumAdapters(0, &dxgiAdapter)
		);

	ThrowIfFailed(
		dxgiAdapter->EnumOutputs(0, &m_dxgiOutput)
		);

	// Create ink stroke style.
	ThrowIfFailed(
		m_d2dFactory->CreateStrokeStyle(
		StrokeStyleProperties(
		D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE_ROUND,
		D2D1_LINE_JOIN_ROUND,
		1.0f,
		D2D1_DASH_STYLE_SOLID,
		0.0f),
		nullptr,
		0,
		&m_inkStrokeStyle)
		);

	// Create ink stroke brush.
	ThrowIfFailed(
		m_d2dContext->CreateSolidColorBrush(ConvertToColorF(BrushColor), &m_strokeBrush)
		);

	m_loadingComplete = true;
}

void DrawingPanel::CreateSizeDependentResources()
{
	m_currentBuffer.Reset();
	m_previousBuffer.Reset();

	DirectXPanelBase::CreateSizeDependentResources();

	ThrowIfFailed(
		m_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		&m_currentBuffer
		)
		);

	ThrowIfFailed(
		m_swapChain->GetBuffer(
		1,
		__uuidof(ID3D11Texture2D),
		&m_previousBuffer
		)
		);
}

void DrawingPanel::Render()
{
	if (!m_loadingComplete || m_timer.GetFrameCount() == 0)
	{
		return;
	}

	// Render and present the DirectX content.

	if (m_drawingState == DrawingState::Ready)
	{
		m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]()
		{
			m_d2dContext->BeginDraw();

			m_d2dContext->Clear(m_backgroundColor);

			for (int channelIndex = m_NumberActiveChannels; channelIndex != 0; channelIndex--)
			{
				if (m_channelVisibility[channelIndex-1]){
					ComPtr<ID2D1SolidColorBrush> strokeBrush;
					ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(DX::ConvertToColorF(m_brushColors[channelIndex-1]), &strokeBrush));

					critical_section::scoped_lock lock(m_criticalSection);

					ComPtr<ID2D1PathGeometry> chartLineGeometry;
					ThrowIfFailed(m_d2dFactory->CreatePathGeometry(&chartLineGeometry));
					ComPtr<ID2D1GeometrySink> sink;
					ThrowIfFailed((chartLineGeometry)->Open(&sink));

					sink->SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);
					sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

					sink->BeginFigure(m_ChartPoints[0][channelIndex-1], D2D1_FIGURE_BEGIN_FILLED);
					for (int idx = 0; idx < m_NumberPoints[channelIndex-1]; ++idx)
						sink->AddLine(m_ChartPoints[idx][channelIndex-1]);

					sink->EndFigure(D2D1_FIGURE_END_OPEN);

					ThrowIfFailed(sink->Close());

					m_d2dContext->DrawGeometry(chartLineGeometry.Get(), strokeBrush.Get(), 1.5f, m_inkStrokeStyle.Get());
				}
			}


			HRESULT hr = m_d2dContext->EndDraw();
			// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
			// is lost. It will be handled during the next call to Present.
			if (hr != D2DERR_RECREATE_TARGET)
			{
				ThrowIfFailed(hr);
			}

			Present();
		}));
	}
}

void DrawingPanel::Update()
{
	critical_section::scoped_lock lock(m_criticalSection);

	Render();
}


void DrawingPanel::SetChannelColor(int channelIndex, Windows::UI::Color color)
{
	m_brushColors[channelIndex] = color;
}

void DrawingPanel::SetNumberChannels(int numberChannels) {
	m_NumberActiveChannels = numberChannels;
}

void DrawingPanel::SetChannelVisibility(int channelIndex, bool visibility) {
	m_channelVisibility[channelIndex] = visibility;
}

// Loads saved strokes from the specified stream and redraws them over time in the order they were drawn.
void DrawingPanel::LoadFromStream(int channelIndex, IRandomAccessStream^ stream, int numberPoints)
{
	if (m_drawingState == DrawingState::Uninitialized)
		return;

	DataReader ^rdr = ref new DataReader(stream);
	create_task(rdr->LoadAsync(numberPoints * sizeof(double) * 2)).then([=](task<unsigned int> bytesLoaded)
	{
		size_t result = bytesLoaded.get();

		m_NumberPoints[channelIndex] = numberPoints;
		for (int idx = 0; idx < numberPoints; ++idx)
		{			
			double x = rdr->ReadDouble();
			double y = rdr->ReadDouble();
			m_ChartPoints[idx][channelIndex] = (D2D1::Point2F(x, y));
		}
		
		m_drawingState = DrawingState::Ready;
	});
}

// Stops replaying current strokes.
void DrawingPanel::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Create a task that will be run on a background thread.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{			
			m_timer.Tick([&]()
			{
				Render();
			});

			// Halt the thread until the next vblank is reached.
			// This ensures the app isn't updating and rendering faster than the display can refresh,
			// which would unnecessarily spend extra CPU and GPU resources.  This helps improve battery life.
			m_dxgiOutput->WaitForVBlank();
		}
	});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DrawingPanel::StopRenderLoop()
{

}


void DrawingPanel::OnDeviceLost()
{
	// Handle device lost, then re-render the current frame if resources are initialized and the user isn't drawing or replaying.
	DirectXPanelBase::OnDeviceLost();

	if (m_drawingState == DrawingState::None)
	{
		Render();
	}
}

void DrawingPanel::OnSizeChanged(Platform::Object^ sender, SizeChangedEventArgs^ e)
{
	// Process SizeChanged event, then re-render at the new size if resources are initialized and the user isn't drawing or replaying.
	DirectXPanelBase::OnSizeChanged(sender, e);

	if (m_drawingState == DrawingState::None)
	{
		Update();
	}
}

void DrawingPanel::OnCompositionScaleChanged(SwapChainPanel ^sender, Object ^args)
{
	// Process CompositionScaleChanged event, then re-render at the new scale if resources are initialized and the user isn't drawing or replaying.
	DirectXPanelBase::OnCompositionScaleChanged(sender, args);

	if (m_drawingState == DrawingState::None)
	{
		Update();
	}
}

void DrawingPanel::OnResuming(Object^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_criticalSection);

	if (m_drawingState == DrawingState::None)
	{
		Update();
	}
}
