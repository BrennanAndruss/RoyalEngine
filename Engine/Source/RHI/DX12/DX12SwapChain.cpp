#include "Engine/RHI/DX12/DX12SwapChain.h"

#include "Engine/RHI/DX12/DX12Device.h"

#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace Royal::RHI
{
	static void ThrowIfFailed(HRESULT hr, const char* msg)
	{
		if (FAILED(hr))
		{
			throw std::runtime_error(msg);
		}
	}

	DX12SwapChain::DX12SwapChain(DX12Device& device, HWND hwnd, uint32_t width, uint32_t height)
		: m_device(device)
	{
		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = kFrameCount;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(m_device.GetFactory()->CreateSwapChainForHwnd(
			m_device.GetCommandQueue(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1), "Failed to create swap chain.");

		ThrowIfFailed(swapChain1.As(&m_swapChain), "Failed to cast swap chain to IDXGISwapChain1.");
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		CreateRTVHeap();
		CreateRenderTargetViews();
	}

	DX12SwapChain::~DX12SwapChain() = default;

	void DX12SwapChain::CreateRTVHeap()
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = kFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		
		ThrowIfFailed(m_device.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)), "Failed to create RTV descriptor heap.");

		m_rtvDescriptorSize = m_device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	void DX12SwapChain::CreateRenderTargetViews()
	{
		// Create frame resources.
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

		// Create a RTV for each frame.
		for (uint32_t i = 0; i < kFrameCount; i++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])), "Failed to get swap chain buffer.");
			m_device.GetDevice()->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += m_rtvDescriptorSize;
		}
	}

	void DX12SwapChain::ReleaseBackBuffers()
	{
		for (auto& buffer : m_backBuffers)
		{
			buffer.Reset();
		}
	}

	void DX12SwapChain::Resize(uint32_t width, uint32_t height)
	{
		ReleaseBackBuffers();

		ThrowIfFailed(m_swapChain->ResizeBuffers(kFrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0), "Failed to resize swap chain bufffers.");

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		CreateRenderTargetViews();
	}

	void DX12SwapChain::Present()
	{
		ThrowIfFailed(m_swapChain->Present(1, 0), "Failed to present swap chain.");
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12SwapChain::GetCurrentRTV() const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += static_cast<SIZE_T>(m_frameIndex) * m_rtvDescriptorSize;
		return rtvHandle;
	}
}