#include "Engine/RHI/DX12/DX12GpuProfiler.h"

#include "Engine/Core/Assert.h"
#include "Engine/RHI/DX12/DX12Device.h"

#include <directx/d3dx12.h>

namespace Royal::RHI
{
	DX12GpuProfiler::DX12GpuProfiler(DX12Device& device, uint32_t maxEvents)
		: m_device(device), m_maxEvents(maxEvents)
	{
		// Allocate for two timestamps per event (start + end).
		D3D12_QUERY_HEAP_DESC heapDesc{};
		heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		heapDesc.Count = maxEvents * 2;
		m_device.GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&m_queryHeap));

		// Readback buffer sized for all timestamps as UINT64s.
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint64_t) * maxEvents * 2);
		m_device.GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_readbackBuffer));

		m_device.GetCommandQueue()->GetTimestampFrequency(&m_timestampFrequency);

		m_eventNames.resize(maxEvents);
	}

	void DX12GpuProfiler::BeginFrame(ID3D12GraphicsCommandList* commandList)
	{
		m_nextQueryIndex = 0;
	}

	uint32_t DX12GpuProfiler::BeginEvent(ID3D12GraphicsCommandList* commandList, std::string name)
	{
		ROYAL_ASSERT(m_nextQueryIndex < m_maxEvents, "Exceeded max GPU profiler events per frame.");

		uint32_t index = m_nextQueryIndex++;
		m_eventNames[index] = std::move(name);
		commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index * 2);
		return index;
	}

	void DX12GpuProfiler::EndEvent(ID3D12GraphicsCommandList* commandList, uint32_t index)
	{
		commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, index * 2 + 1);
	}

	void DX12GpuProfiler::EndFrame(ID3D12GraphicsCommandList* commandList)
	{
		uint32_t usedQueries = m_nextQueryIndex * 2;
		if (usedQueries == 0) return;

		commandList->ResolveQueryData(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, usedQueries, m_readbackBuffer.Get(), 0);
	}

	void DX12GpuProfiler::Readback()
	{
		m_samples.clear();
		if (m_nextQueryIndex == 0) return;

		uint64_t* data = nullptr;
		CD3DX12_RANGE readRange(0, sizeof(uint64_t) * m_nextQueryIndex * 2);
		if (FAILED(m_readbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&data))))
		{
			return;
		}

		for (uint32_t i = 0; i < m_nextQueryIndex; i++)
		{
			uint64_t start = data[i * 2];
			uint64_t end = data[i * 2 + 1];
			float ms = static_cast<float>(end - start) / static_cast<float>(m_timestampFrequency) * 1000.0f;
			m_samples.push_back({ m_eventNames[i], ms });
		}

		m_readbackBuffer->Unmap(0, nullptr);
	}
}