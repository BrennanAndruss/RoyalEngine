#include "Engine/RHI/DX12/DX12Device.h"

#include "Engine/Core/Logger.h"

#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace Royal::RHI
{
	static void ThrowIfFailed(HRESULT hr, const char* msg)
	{
		if (FAILED(hr))
		{
			ROYAL_LOG_FATAL("{}, hr={:#x}", msg, static_cast<unsigned>(hr));
			throw std::runtime_error(msg);
		}
	}

	DX12Device::DX12Device()
	{
		EnableDebugLayer();
		CreateFactory();
		SelectAdapter();
		CreateDevice();
		CreateCommandQueue();
	}

	DX12Device::~DX12Device() = default;

	void DX12Device::EnableDebugLayer()
	{
#if defined(_DEBUG)
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
		{
			m_debugController->EnableDebugLayer();
		}
#endif
	}

	void DX12Device::CreateFactory()
	{
		UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)), "Failed to create DXGI factory.");
	}

	void DX12Device::SelectAdapter()
	{
		// Pick the discrete GPU when one exists.
		for (UINT i = 0; m_factory->EnumAdapterByGpuPreference(
			i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter))!= DXGI_ERROR_NOT_FOUND; 
			i++)
		{
			DXGI_ADAPTER_DESC1 desc{};
			m_adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Skip the WARP software adapter.
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			{
				return;
			}
		}
	}

	void DX12Device::CreateDevice()
	{
		ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "Failed to create D3D12 device.");
	}

	void DX12Device::CreateCommandQueue()
	{
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		ThrowIfFailed(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)), "Failed to create command queue.");
	}
}