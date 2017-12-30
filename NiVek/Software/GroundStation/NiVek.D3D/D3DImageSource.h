
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "pch.h"
#include "d2d1.h"
#include "V3DHeader.h"

#define MAXMESHES 10

namespace NiVek
{
    struct ModelViewProjectionConstantBuffer
    {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    struct VertexPositionColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 color;
    };

    public ref class D3DImageSource sealed : Windows::UI::Xaml::Media::Imaging::SurfaceImageSource
    {
    public:
		D3DImageSource(int pixelWidth, int pixelHeight, bool isOpaque, Platform::String ^modelName);
		D3DImageSource(int pixelWidth, int pixelHeight, bool isOpaque);

        void BeginDraw();
        void EndDraw();
		void SetScaling(float scale);
		void SetLocation(float x, float y, float z);		
        void Clear(Windows::UI::Color color);
        void D3DImageSource::Render(float pitch, float yaw, float roll);

    private protected:
        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
		uint8 m_totalMeshes;
		uint8 m_meshesLoaded;

		float m_locationX;
		float m_locationY;
		float m_locationZ;

		float m_scaling;

		std::vector<VSD3DStarter::Mesh*> m_meshModels;

		VSD3DStarter::Graphics m_graphics;
		VSD3DStarter::LightConstants m_lightConstants;
		VSD3DStarter::MiscConstants m_miscConstants;

        Microsoft::WRL::ComPtr<ISurfaceImageSourceNative>   m_sisNative;
		D3D_FEATURE_LEVEL m_d3dFeatureLevel;
    
        // Direct3D objects
	    Microsoft::WRL::ComPtr<ID2D1Factory1>               m_d2dFactory; 
        Microsoft::WRL::ComPtr<ID3D11Device>                m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext>         m_d3dContext;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_renderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>           m_inputLayout;        
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_vertexBuffer[MAXMESHES];
        Microsoft::WRL::ComPtr<ID3D11Buffer>                m_indexBuffer[MAXMESHES];
        Microsoft::WRL::ComPtr<ID3D11Buffer>                m_constantBuffer;

		Platform::String^									m_modelName;	


        ModelViewProjectionConstantBuffer                   m_constantBufferData;

		uint32                                              m_indexCount[MAXMESHES];

        int                                                 m_width;
        int                                                 m_height;

        float                                               m_frameCount;
    };
}
