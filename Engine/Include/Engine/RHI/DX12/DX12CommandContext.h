#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

namespace Royal::RHI
{
	class DX12Device;

	class DX12CommandContext
	{
	public:
		explicit DX12CommandContext(DX12Device& device);
		~DX12CommandContext();

		DX12CommandContext(const DX12CommandContext&) = delete;
		DX12CommandContext& operator=(const DX12CommandContext&) = delete;

		void Reset();
		ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
		
		// Submits recorded commands and blocks until GPU is done.
		void ExecuteAndWait();

		// Blocks until the last submitted fence value is reached.
		void WaitForGPU();

	private:
		DX12Device& m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent = nullptr;
		uint64_t m_fenceValue = 0;
		bool m_isRecording = false;
	};
}