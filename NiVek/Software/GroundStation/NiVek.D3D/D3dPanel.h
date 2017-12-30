#pragma once

#include "pch.h"
#include "DirectXPanelBase.h"
#include "StepTimer.h"
#include "ShaderStructures.h"
#include "V3DHeader.h"

namespace NiVek
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class D3DPanel sealed : public NiVek::DirectXPanelBase
	{
	public:
		D3DPanel();

		void StartRenderLoop();
		void StopRenderLoop();
		void LoadModels();
		void SetRotation(float pitch, float roll, float yaw);
		void SetLocation(float x, float y, float z);
		void SetScaling(float scaling);

	private protected:

		virtual void Render() override;
		virtual void CreateDeviceResources() override;
		virtual void CreateSizeDependentResources() override;

		std::vector<VSD3DStarter::Mesh*> m_meshModels;

		VSD3DStarter::Graphics m_graphics;
		VSD3DStarter::LightConstants m_lightConstants;
		VSD3DStarter::MiscConstants m_miscConstants;

		Microsoft::WRL::ComPtr<IDXGIOutput>                 m_dxgiOutput;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>           m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                m_constantBuffer;

		DX::ModelViewProjectionConstantBuffer               m_constantBufferData;

		uint32                                              m_indexCount;

		float	                                            m_degreesPerSecond;
		float												m_scaling;
		float												m_pitch;
		float												m_roll;
		float												m_yaw;

		float												m_x;
		float												m_y;
		float												m_z;

		Windows::Foundation::IAsyncAction^					m_renderLoopWorker;
		// Rendering loop timer.
		DX::StepTimer                                       m_timer;

	private:
		~D3DPanel();
	};
}

