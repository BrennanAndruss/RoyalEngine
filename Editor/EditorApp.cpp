#include "EditorApp.h"

#include "Engine/Platform/Window.h"
#include "Engine/RHI/DX12/DX12Device.h"
#include "Engine/RHI/DX12/DX12SwapChain.h"
#include "Engine/RHI/DX12/DX12CommandContext.h"
#include "TriangleVertex.h"

#include <directx/d3dx12.h>
#include <fstream>
#include <stdexcept>
#include <vector>

using Microsoft::WRL::ComPtr;

std::vector<uint8_t> ReadFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		throw std::runtime_error("Failed to open shader file: " + path);
	}

	size_t size = static_cast<size_t>(file.tellg());
	std::vector<uint8_t> buffer(size);
	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	return buffer;
}

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

	GetWindow().SetResizeCallback([this](uint32_t width, uint32_t height) { OnWindowResize(width, height); });

	CreateRootSignature();
	CreatePipelineState();
	CreateVertexBuffer();
}

void EditorApp::OnUpdate(float deltaTime)
{

}

void EditorApp::OnRender()
{
	m_commandContext->Reset();
	ID3D12GraphicsCommandList* commandList = m_commandContext->GetCommandList();

	// Set necessary state.
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	commandList->SetPipelineState(m_pipelineState.Get());

	D3D12_VIEWPORT viewport{ 0.0f, 0.0f,
	static_cast<float>(GetWindow().GetWidth()), static_cast<float>(GetWindow().GetHeight()), 0.0f, 1.0f };
	D3D12_RECT scissor{ 0, 0, static_cast<LONG>(GetWindow().GetWidth()), static_cast<LONG>(GetWindow().GetHeight()) };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	ID3D12Resource* backBuffer = m_swapChain->GetCurrentBackBuffer();

	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER toRT = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &toRT);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain->GetCurrentRTV();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	
	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);

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

void EditorApp::OnWindowResize(uint32_t width, uint32_t height)
{
	// Swap chain buffers must fully idle before they can be resized.
	m_commandContext->WaitForGPU();
	m_swapChain->Resize(width, height);
}

void EditorApp::CreateRootSignature()
{
	// Create an empty root signature.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			throw std::runtime_error(static_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		throw std::runtime_error("Failed to serialize root signature.");
	}

	if (FAILED(m_device->GetDevice()->CreateRootSignature(0, 
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature))))
	{
		throw std::runtime_error("Failed to create root signature.");
	}
}

void EditorApp::CreatePipelineState()
{
	std::vector<uint8_t> vertexShaderBytes = ReadFile("Shaders/Triangle_VS.cso");
	std::vector<uint8_t> pixelShaderBytes = ReadFile("Shaders/Triangle_PS.cso");

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vertexShaderBytes.data(), vertexShaderBytes.size() };
	psoDesc.PS = { pixelShaderBytes.data(), pixelShaderBytes.size() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	if (FAILED(m_device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState))))
	{
		throw std::runtime_error("Failed to create pipeline state.");
	}
}

void EditorApp::CreateVertexBuffer()
{
	TriangleVertex vertices[] = {
	{ { 0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
	{ {-0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
	};
	const UINT vertexBufferSize = sizeof(vertices);

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	if (FAILED(m_device->GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)
	)))
	{
		throw std::runtime_error("Failed to create vertex buffer.");
	}

	// Copy the triangle data to the vertex buffer.
	void* vertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	m_vertexBuffer->Map(0, &readRange, &vertexDataBegin);
	memcpy(vertexDataBegin, vertices, vertexBufferSize);
	m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(TriangleVertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
}