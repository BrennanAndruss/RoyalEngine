#include "EditorApp.h"

#include "Engine/Platform/Window.h"
#include "Engine/RHI/DX12/DX12Device.h"
#include "Engine/RHI/DX12/DX12SwapChain.h"
#include "Engine/RHI/DX12/DX12CommandContext.h"

#include <directx/d3dx12.h>

EditorApp::EditorApp(const ApplicationConfig& config)
	: Application(config)
{

}

EditorApp::~EditorApp() = default;

void EditorApp::OnInit()
{
	m_device = std::make_unique<RHI::DX12Device>();
	m_swapChain = std::make_unique<RHI::DX12SwapChain>(
		*m_device, GetWindow().GetHandle(), GetWindow().GetWidth(), GetWindow().GetHeight()
	);
	m_commandContext = std::make_unique<RHI::DX12CommandContext>(*m_device);
}

void EditorApp::OnUpdate(float deltaTime)
{

}

void EditorApp::OnRender()
{
	m_commandContext->Reset();
	ID3D12GraphicsCommandList* commandList = m_commandContext->GetCommandList();

	ID3D12Resource* backBuffer = m_swapChain->GetCurrentBackBuffer();

	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER toRT = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &toRT);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain->GetCurrentRTV();
	
	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Indicate that the back buffer will now be used to present.
	D3D12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &toPresent);

	m_commandContext->ExecuteAndWait();
	m_swapChain->Present();
}

void EditorApp::OnShutdown()
{
	m_commandContext.reset();
	m_swapChain.reset();
	m_device.reset();
}