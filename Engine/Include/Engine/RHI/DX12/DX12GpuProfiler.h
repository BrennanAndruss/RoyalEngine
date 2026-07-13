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
		DX12GpuProfiler(DX12Device& device, uint32_t maxEvents = 32);

		void BeginFrame(ID3D12GraphicsCommandList* commandList);
		uint32_t BeginEvent(ID3D12GraphicsCommandList* commandList, std::string name);
		void EndEvent(ID3D12GraphicsCommandList* commandList, uint32_t index);
		void EndFrame(ID3D12GraphicsCommandList* commandList);

		void Readback();

		const std::vector<GPUSample>& GetLastFrameSamples() const { return m_samples; }

	private:
		DX12Device& m_device;
		Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_queryHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;
		std::vector<std::string> m_eventNames;
		uint32_t m_maxEvents;
		uint32_t m_nextQueryIndex = 0;
		uint64_t m_timestampFrequency = 1;
		std::vector<GPUSample> m_samples;
	};
}