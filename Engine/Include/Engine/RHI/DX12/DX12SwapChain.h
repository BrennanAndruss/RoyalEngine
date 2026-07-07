#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <array>
#include <cstdint>

namespace Royal::RHI
{
	class DX12Device;

	class DX12SwapChain
	{
	public:
		DX12SwapChain(DX12Device& device, HWND hwnd, uint32_t width, uint32_t height);
		~DX12SwapChain();

		DX12SwapChain(const DX12SwapChain&) = delete;
		DX12SwapChain& operator=(const DX12SwapChain&) = delete;

		void Resize(uint32_t width, uint32_t height);
		void Present();

		ID3D12Resource* GetCurrentBackBuffer() const { return m_backBuffers[m_frameIndex].Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
		uint32_t GetCurrentFrameIndex() const { return m_frameIndex; }

		static constexpr uint32_t kFrameCount = 2;

	private:
		void CreateRTVHeap();
		void CreateRenderTargetViews();
		void ReleaseBackBuffers();

		DX12Device& m_device;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kFrameCount> m_backBuffers;
		uint32_t m_rtvDescriptorSize = 0;
		uint32_t m_frameIndex = 0;
	};
}