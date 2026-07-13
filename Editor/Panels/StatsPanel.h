#pragma once

#include "Engine/RHI/DX12/DX12GpuProfiler.h"

#include <vector>

namespace Royal { class Window; }
namespace Royal::RHI { struct GPUSample; }

namespace Editor::Panels
{
	void DrawStatsPanel(bool& open, const Royal::Window& window, 
		const std::vector<Royal::RHI::GPUSample>& gpuSamples);
}