#pragma once

#include "Engine/Core/Application.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>

namespace Royal::RHI
{
	class DX12Device;
	class DX12SwapChain;
	class DX12CommandContext;
}

using namespace Royal;

class EditorApp : public Application
{
public:
	explicit EditorApp(const ApplicationConfig& config);
	~EditorApp();

protected:
	void OnInit() override;
	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void OnWindowResize(uint32_t width, uint32_t height);

	// Pipeline objects.
	std::unique_ptr<RHI::DX12Device> m_device;
	std::unique_ptr<RHI::DX12SwapChain> m_swapChain;
	std::unique_ptr<RHI::DX12CommandContext> m_commandContext;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	// App resources.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};

	void CreateRootSignature();
	void CreatePipelineState();
	void CreateVertexBuffer();
};