#include "pch.h"
#include "D2DWrapper.h"

using namespace NiVek;
using namespace Platform;


using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace D2D1;
using namespace Windows::Storage;
using namespace Platform;

using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Storage::Streams;

void NiVek::D2DWrapper::Initialize( Windows::UI::Core::CoreWindow^ window )
{
	m_window = window;
	m_displayInfo = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
	
	m_dpi = static_cast<float>(m_displayInfo->LogicalDpi);
	m_renderTargetBitmap = nullptr;
	CreateDeviceIndependentResources();
	CreateDeviceResources();
	CreateWindowSizeDependentResources();
}

void NiVek::D2DWrapper::CreateDeviceIndependentResources()
{
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	DX::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &m_d2dFactory));

	DX::ThrowIfFailed(DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwriteFactory ) );

	DX::ThrowIfFailed(CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory)));
}

void NiVek::D2DWrapper::CreateDeviceResources()
{
	// This flag adds support for surfaces with a different color channel ordering than the API default.
	// It is recommended usage, and is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers with this flag.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the DX11 API device object, and get a corresponding context.
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dContext;
	DX::ThrowIfFailed(
		D3D11CreateDevice(
		nullptr,                  // specify null to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,                  // leave as nullptr unless software device
		creationFlags,            // optionally set debug and Direct2D compatibility flags
		featureLevels,            // list of feature levels this app can support
		ARRAYSIZE(featureLevels), // number of entries in above list
		D3D11_SDK_VERSION,        // always set this to D3D11_SDK_VERSION for modern
		&d3dDevice,               // returns the Direct3D device created
		&m_featureLevel,          // returns feature level of device created
		&d3dContext               // returns the device immediate context
		)
		);

	// Get the DirectX11.1 device by QI off the DirectX11 one.
	DX::ThrowIfFailed(
		d3dDevice.As(&m_d3dDevice)
		);

	// And get the corresponding device context in the same way.
	DX::ThrowIfFailed(
		d3dContext.As(&m_d3dContext)
		);

	// Obtain the underlying DXGI device of the Direct3D11.1 device.
	ComPtr<IDXGIDevice> dxgiDevice;
	DX::ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);

	// Obtain the Direct2D device for 2D rendering.
	DX::ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
		);

	// And get its corresponding device context object.
	DX::ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m_d2dContext
		)
		);

	DX::ThrowIfFailed(
		m_dwriteFactory->CreateTextFormat(
		L"Segoe UI",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		58.0f,
		L"en-US",
		&nameTextFormat
		)
		);

	DX::ThrowIfFailed(
		nameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
		);

	DX::ThrowIfFailed(
		nameTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
		);


	// Release the swap chain (if it exists) as it will be incompatible with
	// the new device.
	m_swapChain = nullptr;
}

void NiVek::D2DWrapper::CreateWindowSizeDependentResources()
{
	// Store the window bounds so the next time we get a SizeChanged event we can
	// avoid rebuilding everything if the size is identical.
	m_windowBounds = m_window->Bounds;

	// If the swap chain already exists, resize it.
	if(m_swapChain != nullptr)
	{
		DX::ThrowIfFailed(
			m_swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0)
			);
	}
	// Otherwise, create a new one.
	else
	{
		// Create a descriptor for the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = 0;                                     // use automatic sizing
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swapchain format
		swapChainDesc.Stereo = false; 
		swapChainDesc.SampleDesc.Count = 1;                          // don't use multi-sampling
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;                               // use double buffering to enable flip
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
		swapChainDesc.Flags = 0;

		// Once the desired swap chain description is configured, it must be created on the same adapter as our D3D Device

		// First, retrieve the underlying DXGI Device from the D3D Device.
		ComPtr<IDXGIDevice1> dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		// Identify the physical adapter (GPU or card) this device is running on.
		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		// And obtain the factory object that created it.
		ComPtr<IDXGIFactory2> dxgiFactory;
		DX::ThrowIfFailed(
			dxgiAdapter->GetParent(
			__uuidof(IDXGIFactory2), 
			&dxgiFactory
			)
			);

		// Create a swap chain for this window from the DXGI factory.
		DX::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForCoreWindow(
			m_d3dDevice.Get(),
			reinterpret_cast<IUnknown*>(m_window),
			&swapChainDesc,
			nullptr,    // allow on all displays
			&m_swapChain
			)
			);

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces 
		// latency and ensures that the application will only render after each VSync, minimizing 
		// power consumption.
		DX::ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);

	}

	// Obtain the backbuffer for this window which will be the final 3D rendertarget.
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		&backBuffer
		)
		);

	// Create a view interface on the rendertarget to use on bind.
	DX::ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		&m_renderTargetView
		)
		);

	// Cache the rendertarget dimensions in our helper class for convenient use.
	D3D11_TEXTURE2D_DESC backBufferDesc = {0};
	backBuffer->GetDesc(&backBufferDesc);
	m_renderTargetSize.Width  = static_cast<float>(backBufferDesc.Width);
	m_renderTargetSize.Height = static_cast<float>(backBufferDesc.Height);

	// Create a descriptor for the depth/stencil buffer.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT, 
		backBufferDesc.Width,
		backBufferDesc.Height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
		);

	// Allocate a 2-D surface as the depth/stencil buffer.
	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&depthStencil
		)
		);

	// Create a DepthStencil view on this surface to use on bind.
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D),
		&m_depthStencilView
		)
		);

	// Create a viewport descriptor of the full window size.
	CD3D11_VIEWPORT viewport(
		0.0f,
		0.0f,
		static_cast<float>(backBufferDesc.Width),
		static_cast<float>(backBufferDesc.Height)
		);

	// Set the current viewport using the descriptor.
	m_d3dContext->RSSetViewports(1, &viewport);

	// Now we set up the Direct2D render target bitmap linked to the swapchain. 
	// Whenever we render to this bitmap, it will be directly rendered to the 
	// swapchain associated with the window.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
		m_dpi,
		m_dpi
		);

	// Direct2D needs the dxgi version of the backbuffer surface pointer.
	ComPtr<IDXGISurface> dxgiBackBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(
		0,
		__uuidof(IDXGISurface),
		&dxgiBackBuffer
		)
		);

	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	DX::ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer.Get(),
		&bitmapProperties,
		&m_d2dTargetBitmap
		)
		);

	// So now we can set the Direct2D render target.
	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());

	// Set D2D text anti-alias mode to Grayscale to ensure proper rendering of text on intermediate surfaces.
	m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void NiVek::D2DWrapper::UpdateForWindowSizeChange()
{

}

/*void NiVek::D2DWrapper::SetDpi( float dpi )
{

}*/


void NiVek::D2DWrapper::SaveBitmapToFile()
{

}

void NiVek::D2DWrapper::SaveBitmapToStream( _In_ ComPtr<ID2D1Bitmap1> d2dBitmap, 
														 _In_ ComPtr<IWICImagingFactory2> wicFactory2, 
														 _In_ ComPtr<ID2D1DeviceContext> d2dContext, 
														 _In_ REFGUID wicFormat, 
														 _In_ IStream* stream ){
	// Create and initialize WIC Bitmap Encoder.
	ComPtr<IWICBitmapEncoder> wicBitmapEncoder;
	DX::ThrowIfFailed(
		wicFactory2->CreateEncoder(
		wicFormat,
		nullptr,    // No preferred codec vendor.
		&wicBitmapEncoder
		)
		);

	DX::ThrowIfFailed(
		wicBitmapEncoder->Initialize(
		stream,
		WICBitmapEncoderNoCache
		)
		);

	// Create and initialize WIC Frame Encoder.
	ComPtr<IWICBitmapFrameEncode> wicFrameEncode;
	DX::ThrowIfFailed(
		wicBitmapEncoder->CreateNewFrame(
		&wicFrameEncode,
		nullptr     // No encoder options.
		)
		);

	DX::ThrowIfFailed(
		wicFrameEncode->Initialize(nullptr)
		);

	// Retrieve D2D Device.
	ComPtr<ID2D1Device> d2dDevice;
	d2dContext->GetDevice(&d2dDevice);

	// Create IWICImageEncoder.
	ComPtr<IWICImageEncoder> imageEncoder;
	DX::ThrowIfFailed(
		wicFactory2->CreateImageEncoder(
		d2dDevice.Get(),
		&imageEncoder
		)
		);

	DX::ThrowIfFailed(
		imageEncoder->WriteFrame(
		d2dBitmap.Get(),
		wicFrameEncode.Get(),
		nullptr     // Use default WICImageParameter options.
		)
		);

	DX::ThrowIfFailed(
		wicFrameEncode->Commit()
		);

	DX::ThrowIfFailed(
		wicBitmapEncoder->Commit()
		);

	// Flush all memory buffers to the next-level storage object.
	DX::ThrowIfFailed( stream->Commit(STGC_DEFAULT) );

}

void NiVek::D2DWrapper::BeginDrawChart(int width, int height){
	if(m_renderTargetBitmap == nullptr){
		D2D1_BITMAP_PROPERTIES1 bmpProps = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET,D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED), 96, 96,0);
		DX::ThrowIfFailed( m_d2dContext->CreateBitmap(D2D1::SizeU(width,height),nullptr,0, bmpProps, &m_renderTargetBitmap) );
		m_d2dContext->SetTarget(m_renderTargetBitmap.Get());
	}

	m_d2dContext->BeginDraw();

	ColorF brushColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);

	m_d2dContext->Clear(brushColor);
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

void NiVek::D2DWrapper::DrawLine(const Platform::Array<float>^ points, int pointCount, Windows::UI::Color color){
	ID2D1SolidColorBrush *brush;

	DX::ThrowIfFailed( m_d2dContext->CreateSolidColorBrush(DX::ConvertToColorF(color), &brush) );	

	for(int idx = 0; idx < pointCount - 2; idx += 2){
		m_d2dContext->DrawLine(D2D1::Point2F(points[idx], points[idx+1]), 
			D2D1::Point2F(points[idx + 2], points[idx+3]),
			brush, 2.0f, 0);
	}

	brush->Release();
}

void NiVek::D2DWrapper::EndDrawChart(){
	m_d2dContext->EndDraw();
}

Windows::Storage::Streams::InMemoryRandomAccessStream^ NiVek::D2DWrapper::GetImageStream(){
	GUID wicFormat = GUID_ContainerFormatBmp;
	ComPtr<IStream> stream;

	ComPtr<ISequentialStream> ss;
	auto m_inMemStream =  ref new InMemoryRandomAccessStream();
	DX::ThrowIfFailed(CreateStreamOverRandomAccessStream(m_inMemStream ,IID_PPV_ARGS(&stream)));
	SaveBitmapToStream(m_renderTargetBitmap, m_wicFactory, m_d2dContext, wicFormat, stream.Get());

	return m_inMemStream;
}
