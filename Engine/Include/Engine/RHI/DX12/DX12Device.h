#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Royal::RHI
{
	class DX12Device
	{
	public:
		DX12Device();
		~DX12Device();

		DX12Device(const DX12Device&) = delete;
		DX12Device& operator=(const DX12Device&) = delete;

		ID3D12Device* GetDevice() const { return m_device.Get(); }
		ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
		IDXGIFactory6* GetFactory() const { return m_factory.Get(); }

	private:
		void EnableDebugLayer();
		void CreateFactory();
		void SelectAdapter();
		void CreateDevice();
		void CreateCommandQueue();

		Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

#if defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> m_debugController;
#endif
	};
}