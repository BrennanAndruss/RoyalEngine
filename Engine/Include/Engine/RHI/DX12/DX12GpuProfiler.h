#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>

namespace Royal::RHI
{
	class DX12Device;

	struct GPUSample
	{
		std::string name;
		float milliseconds;
	};

	class DX12GpuProfiler
	{
	public:
		DX12GpuProfiler(DX12Device& device, uint32_t frameCount, uint32_t maxEventsPerFrame = 32);

		// Reset this slot's event counter.
		void BeginFrame(uint32_t frameIndex);
		uint32_t BeginEvent(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, std::string name);
		void EndEvent(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, uint32_t eventIndex);
		void EndFrame(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex);

		void Readback(uint32_t frameIndex);

		const std::vector<GPUSample>& GetLastFrameSamples() const { return m_lastCompletedSamples; }

	private:
		DX12Device& m_device;
		uint32_t m_frameCount;
		uint32_t m_maxEventsPerFrame;

		Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_queryHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;
		uint64_t m_timestampFrequency = 1;

		std::vector<std::vector<std::string>> m_eventNames;
		std::vector<uint32_t> m_nextQueryIndex;
		std::vector<GPUSample> m_lastCompletedSamples;
	};
}