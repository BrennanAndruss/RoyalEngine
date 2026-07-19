#include "Engine/RHI/DX12/DX12GpuProfiler.h"

#include "Engine/Core/Assert.h"
#include "Engine/RHI/DX12/DX12Device.h"

#include <directx/d3dx12.h>

namespace Royal::RHI
{
	DX12GpuProfiler::DX12GpuProfiler(DX12Device& device, uint32_t frameCount, uint32_t maxEventsPerFrame)
		: m_device(device), m_frameCount(frameCount), m_maxEventsPerFrame(maxEventsPerFrame)
	{
		// Allocate for two timestamps per event (start + end) for each frame.
		uint32_t totalQueries = frameCount * maxEventsPerFrame * 2;

		D3D12_QUERY_HEAP_DESC heapDesc{};
		heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		heapDesc.Count = totalQueries;
		m_device.GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&m_queryHeap));

		// Readback buffer sized for all timestamps as UINT64s.
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint64_t) * totalQueries);
		m_device.GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_readbackBuffer));

		m_device.GetCommandQueue()->GetTimestampFrequency(&m_timestampFrequency);

		m_eventNames.resize(frameCount);
		for (auto& names : m_eventNames)
		{
			names.resize(maxEventsPerFrame);
		}
		m_nextQueryIndex.resize(frameCount, 0);
	}

	void DX12GpuProfiler::BeginFrame(uint32_t frameIndex)
	{
		m_nextQueryIndex[frameIndex] = 0;
	}

	uint32_t DX12GpuProfiler::BeginEvent(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, std::string name)
	{
		ROYAL_ASSERT(m_nextQueryIndex[frameIndex] < m_maxEventsPerFrame, "Exceeded max GPU profiler events per frame.");

		uint32_t eventIndex = m_nextQueryIndex[frameIndex]++;
		m_eventNames[frameIndex][eventIndex] = std::move(name);

		uint32_t slot = (frameIndex * m_maxEventsPerFrame + eventIndex) * 2;
		commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, slot);
		return eventIndex;
	}

	void DX12GpuProfiler::EndEvent(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, uint32_t eventIndex)
	{
		uint32_t slot = (frameIndex * m_maxEventsPerFrame + eventIndex) * 2 + 1;
		commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, slot);
	}

	void DX12GpuProfiler::EndFrame(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex)
	{
		uint32_t usedQueries = m_nextQueryIndex[frameIndex] * 2;
		if (usedQueries == 0) return;

		commandList->ResolveQueryData(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, usedQueries, m_readbackBuffer.Get(), 0);
	}

	void DX12GpuProfiler::Readback(uint32_t frameIndex)
	{
		m_lastCompletedSamples.clear();

		// Get event count from this slot's previous occupant.
		uint32_t eventCount = m_nextQueryIndex[frameIndex];
		if (eventCount == 0) return;

		uint32_t startQuery = frameIndex * m_maxEventsPerFrame * 2;
		uint64_t startBytes = static_cast<uint64_t>(startQuery) * sizeof(uint64_t);
		uint64_t rangeBytes = static_cast<uint64_t>(eventCount) * 2 * sizeof(uint64_t);

		uint64_t* data = nullptr;
		CD3DX12_RANGE readRange(startBytes, startBytes + rangeBytes);
		if (FAILED(m_readbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&data))))
		{
			return;
		}

		for (uint32_t i = 0; i < eventCount; i++)
		{
			uint64_t start = data[startQuery + i * 2];
			uint64_t end = data[startQuery + i * 2 + 1];
			float ms = static_cast<float>(end - start) / static_cast<float>(m_timestampFrequency) * 1000.0f;
			m_lastCompletedSamples.push_back({ m_eventNames[frameIndex][i], ms });
		}

		m_readbackBuffer->Unmap(0, nullptr);
	}
}