#include "Engine/RHI/DX12/DX12CommandContext.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Logger.h"
#include "Engine/RHI/DX12/DX12Device.h"

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

	DX12CommandContext::DX12CommandContext(DX12Device& device)
		: m_device(device)
	{
		ID3D12Device* d3dDevice = m_device.GetDevice();

		for (FrameSlot& slot : m_frameSlots)
		{
			ThrowIfFailed(d3dDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&slot.commandAllocator)), "Failed to create command allocator.");
		}

		// Create the shared command command list, created in the recording state.
		ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_frameSlots[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)), "Failed to create command list.");

		// Close the command list so it can be reset before recording commands.
		m_commandList->Close();

		// Create a fence for GPU synchronization.
		ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed to create fence.");

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent)
		{
			throw std::runtime_error("Failed to create fence event.");
		}
	}

	DX12CommandContext::~DX12CommandContext()
	{
		WaitForGPU();
		if (m_fenceEvent)
		{
			CloseHandle(m_fenceEvent);
		}
	}

	void DX12CommandContext::BeginFrame(uint32_t frameIndex)
	{
		ROYAL_ASSERT(frameIndex < kFrameCount, "Frame index out of range.");

		FrameSlot& slot = m_frameSlots[frameIndex];

		// Wait if this slot's previous occupant hasn't finished on the GPU yet.
		if (slot.fenceValue != 0 && m_fence->GetCompletedValue() < slot.fenceValue)
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(slot.fenceValue, m_fenceEvent), "Failed to set fence event.");
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		ThrowIfFailed(slot.commandAllocator->Reset(), "Failed to reset command allocator.");
		ThrowIfFailed(m_commandList->Reset(slot.commandAllocator.Get(), nullptr), "Failed to reset command list.");
	}

	void DX12CommandContext::Execute(uint32_t frameIndex)
	{
		ROYAL_ASSERT(frameIndex < kFrameCount, "Frame index out of range.");

		ThrowIfFailed(m_commandList->Close(), "Failed to close command list.");

		ID3D12CommandList* commandLists[] = { m_commandList.Get() };
		m_device.GetCommandQueue()->ExecuteCommandLists(1, commandLists);

		// Signal and increment the fence value.
		uint64_t signalValue = m_nextFenceValue++;
		ThrowIfFailed(m_device.GetCommandQueue()->Signal(m_fence.Get(), signalValue), "Failed to signal fence");

		m_frameSlots[frameIndex].fenceValue = signalValue;
	}

	void DX12CommandContext::WaitForGPU()
	{
		// Signal and increment the fence value.
		uint64_t signalValue = m_nextFenceValue++;
		ThrowIfFailed(m_device.GetCommandQueue()->Signal(m_fence.Get(), signalValue), "Failed to signal fence.");

		// Wait until the previous frame is finished.
		if (m_fence->GetCompletedValue() < signalValue)
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(signalValue, m_fenceEvent), "Failed to set fence event.");
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}
}