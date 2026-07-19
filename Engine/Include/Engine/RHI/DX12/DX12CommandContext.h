#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <array>
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

		ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
		
		// Waits if this frame slot's prior GPU work hasn't finished yet,
		// then resets that slot's allocator and the shared command list.
		void BeginFrame(uint32_t frameIndex);

		// Submit and signal a new fence value for this frame slot.
		void Execute(uint32_t frameIndex);

		// Blocks until the last submitted fence value is reached.
		void WaitForGPU();

		static constexpr uint32_t kFrameCount = 2;

	private:
		struct FrameSlot
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
			uint64_t fenceValue = 0;
		};

		DX12Device& m_device;
		std::array<FrameSlot, kFrameCount> m_frameSlots;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent = nullptr;
		uint64_t m_nextFenceValue = 1;
	};
}