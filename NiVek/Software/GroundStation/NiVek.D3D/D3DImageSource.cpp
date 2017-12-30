//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "D3DImageSource.h"
#include "BasicReaderWriter.h"
#include "DXHelper.h"

using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::UI;
using namespace DirectX;
using namespace NiVek;

D3DImageSource::D3DImageSource(int pixelWidth, int pixelHeight, bool isOpaque, Platform::String ^modelName) :
    SurfaceImageSource(pixelWidth, pixelHeight, isOpaque)
{
	m_scaling = 1.0f;
	m_locationX = 0.0f;
	m_locationY = 0.0f;
	m_locationZ = 0.0f;

	m_modelName = modelName;

    m_width = pixelWidth;
    m_height = pixelHeight;

    CreateDeviceIndependentResources();
    CreateDeviceResources(); 
}

D3DImageSource::D3DImageSource(int pixelWidth, int pixelHeight, bool isOpaque) :
    SurfaceImageSource(pixelWidth, pixelHeight, isOpaque)
{
	m_scaling = 1.0f;
	m_locationX = 0.0f;
	m_locationY = 0.0f;
	m_locationZ = 0.0f;

	m_modelName = nullptr;

    m_width = pixelWidth;
    m_height = pixelHeight;

    CreateDeviceIndependentResources();
    CreateDeviceResources();
}

// Initialize resources that are independent of hardware.
void D3DImageSource::CreateDeviceIndependentResources()
{
    // Query for ISurfaceImageSourceNative interface.
    DX::ThrowIfFailed(reinterpret_cast<IUnknown*>(this)->QueryInterface(IID_PPV_ARGS(&m_sisNative)));
}

// Initialize hardware-dependent resources.
void D3DImageSource::CreateDeviceResources()
{
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; 

#if defined(_DEBUG)    
    // If the project is in a debug build, enable debugging via SDK Layers.
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // This array defines the set of DirectX hardware feature levels this app will support.
    // Note the ordering should be preserved.
    // Don't forget to declare your application's minimum required feature level in its
    // description.  All applications are assumed to support 9.1 unless otherwise stated.
    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    DX::ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,nullptr,creationFlags,featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_d3dDevice, nullptr, &m_d3dContext));

	m_graphics.Initialize(m_d3dDevice.Get(), m_d3dContext.Get(), m_d3dFeatureLevel);

	//auto loadVSTask = DX::ReadDataAsync("SimpleVertexShader.cso");
	//auto loadPSTask = DX::ReadDataAsync("SimplePixelShader.cso");


    // Get the Direct3D 11.1 API device.
    ComPtr<IDXGIDevice> dxgiDevice;
    DX::ThrowIfFailed( m_d3dDevice.As(&dxgiDevice) );

    // Associate the DXGI device with the SurfaceImageSource.
    DX::ThrowIfFailed( m_sisNative->SetDevice(dxgiDevice.Get()));

    BasicReaderWriter^ reader = ref new BasicReaderWriter();

    // Load the vertex shader.
    auto vsBytecode = reader->ReadData("SimpleVertexShader.cso");
    DX::ThrowIfFailed( m_d3dDevice->CreateVertexShader( vsBytecode->Data, vsBytecode->Length, nullptr, &m_vertexShader ));

    // Create input layout for vertex shader.
    const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    DX::ThrowIfFailed( m_d3dDevice->CreateInputLayout( vertexDesc, ARRAYSIZE(vertexDesc), vsBytecode->Data, vsBytecode->Length, &m_inputLayout ) );

    // Load the pixel shader.
    auto psBytecode = reader->ReadData("SimplePixelShader.cso");
    DX::ThrowIfFailed( m_d3dDevice->CreatePixelShader( psBytecode->Data, psBytecode->Length, nullptr, &m_pixelShader ) );

    // Create the constant buffer.
    const CD3D11_BUFFER_DESC constantBufferDesc = CD3D11_BUFFER_DESC(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    DX::ThrowIfFailed( m_d3dDevice->CreateBuffer( &constantBufferDesc, nullptr, &m_constantBuffer ) );

    // Calculate the aspect ratio and field of view.
    float aspectRatio = (float)m_width / (float)m_height;
    
    float fovAngleY = 70.0f * XM_PI / 180.0f;
    if (aspectRatio < 1.0f)
    {
        fovAngleY /= aspectRatio;
    }

    // Set right-handed perspective projection based on aspect ratio and field of view.
    m_constantBufferData.projection = XMMatrixTranspose(XMMatrixPerspectiveFovRH(fovAngleY,aspectRatio,0.01f,100.0f));

    // Start animating at frame 0.
    m_frameCount = 0;

	VSD3DStarter::Mesh::LoadFromFile(m_graphics, m_modelName->Data(), L"", L"Folder", m_meshModels);
}

void D3DImageSource::SetScaling(float scale) 
{
	m_scaling = scale;
}

void D3DImageSource::SetLocation(float x, float y, float z)
{
	m_locationX = x;
	m_locationY = y;
	m_locationZ = z;
}

// Begins drawing.
void D3DImageSource::BeginDraw()
{
    POINT offset;
    ComPtr<IDXGISurface> surface;

    // Express target area as a native RECT type.
    RECT updateRectNative; 
    updateRectNative.left = 0;
    updateRectNative.top = 0;
    updateRectNative.right = m_width;
    updateRectNative.bottom = m_height;


    // Begin drawing - returns a target surface and an offset to use as the top left origin when drawing.
    HRESULT beginDrawHR = m_sisNative->BeginDraw(updateRectNative, &surface, &offset);

	DXGI_SURFACE_DESC srfc_desc;
	surface.Get()->GetDesc(&srfc_desc);

    if (beginDrawHR == DXGI_ERROR_DEVICE_REMOVED || beginDrawHR == DXGI_ERROR_DEVICE_RESET)
    {
        // If the device has been removed or reset, attempt to recreate it and continue drawing.
        CreateDeviceResources();
        BeginDraw();
    }
    else
    {
        // Notify the caller by throwing an exception if any other error was encountered.
        DX::ThrowIfFailed(beginDrawHR);
    }

    // QI for target texture from DXGI surface.
    ComPtr<ID3D11Texture2D> d3DTexture;
    surface.As(&d3DTexture);


    // Create render target view.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(d3DTexture.Get(), nullptr, &m_renderTargetView));

    // Set viewport to the target area in the surface, taking into account the offset returned by BeginDraw.
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = static_cast<float>(offset.x);
    viewport.TopLeftY = static_cast<float>(offset.y);
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = D3D11_MIN_DEPTH;
    viewport.MaxDepth = D3D11_MAX_DEPTH;
    m_d3dContext->RSSetViewports(1, &viewport);

    // Create depth/stencil buffer descriptor.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, srfc_desc.Width, srfc_desc.Height,1,1,D3D11_BIND_DEPTH_STENCIL);

    // Allocate a 2-D surface as the depth/stencil buffer.
    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));
	
    // Create depth/stencil view based on depth/stencil buffer.
    const CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);    
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(),&depthStencilViewDesc,&m_depthStencilView));

	XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR vPos = XMLoadFloat3(&pos);

	XMFLOAT3 dir;
    XMStoreFloat3(&dir, XMVector3Normalize(vPos));

    m_lightConstants.ActiveLights = 3;
    m_lightConstants.Ambient =  XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_lightConstants.IsPointLight[0] = false;
    m_lightConstants.LightColor[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
    m_lightConstants.LightDirection[0].x = dir.x;
    m_lightConstants.LightDirection[0].y = dir.y;
    m_lightConstants.LightDirection[0].z = dir.z;
    m_lightConstants.LightDirection[0].w = 0;
    m_lightConstants.LightSpecularIntensity[0].x = 10;
	m_lightConstants.LightSpecularIntensity[0].z = 10;
	m_lightConstants.LightSpecularIntensity[0].y = 10;


	pos = XMFLOAT3(0.0f, 2.0f, -2.0f);
	vPos = XMLoadFloat3(&pos);

	XMStoreFloat3(&dir, XMVector3Normalize(vPos));

    m_lightConstants.IsPointLight[1] = false;
    m_lightConstants.LightColor[1] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
    m_lightConstants.LightDirection[1].x = dir.x;
    m_lightConstants.LightDirection[1].y = dir.y;
    m_lightConstants.LightDirection[1].z = dir.z;
    m_lightConstants.LightDirection[1].w = 0;
    m_lightConstants.LightSpecularIntensity[1].x = 2;


	pos = XMFLOAT3(0.0f, 0.0f, 5.0f);
	vPos = XMLoadFloat3(&pos);

	XMStoreFloat3(&dir, XMVector3Normalize(vPos));

    m_lightConstants.IsPointLight[2] = false;
    m_lightConstants.LightColor[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
    m_lightConstants.LightDirection[2].x = dir.x;
    m_lightConstants.LightDirection[2].y = dir.y;
    m_lightConstants.LightDirection[2].z = dir.z;
    m_lightConstants.LightDirection[2].w = 0;
    m_lightConstants.LightSpecularIntensity[2].x = 2;

    m_graphics.UpdateLightConstants(m_lightConstants);
}

// Clears the surface with the given color.  This must be called 
// after BeginDraw and before EndDraw for a given update.
void D3DImageSource::Clear(Color color)
{
    // Convert color values.
    const float clearColor[4] = { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f };
    // Clear render target view with given color.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(),clearColor);
}

// Clears the surface with the given color.  This must be called
// after BeginDraw and before EndDraw for a given update
void D3DImageSource::Render(float pitch, float yaw, float roll)
{
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	XMMATRIX translation = XMMatrixTranslation(m_locationX,m_locationY,m_locationZ);

	XMMATRIX scale = XMMatrixScaling(m_scaling,m_scaling,m_scaling) * rotation;

	XMMATRIX world = scale * translation;

    for (UINT i = 0; i < m_meshModels.size(); i++)
    {

        m_miscConstants.Time =30;// m_time[i];
        m_graphics.UpdateMiscConstants(m_miscConstants);
        m_meshModels[i]->Render(m_graphics, world);
    }
}

// Ends drawing updates started by a previous BeginDraw call.
void D3DImageSource::EndDraw()
{
    DX::ThrowIfFailed(
        m_sisNative->EndDraw()
        );
}
