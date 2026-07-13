#include "EditorApp.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/CVar.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/Logger.h"
#include "Engine/Platform/Window.h"
#include "Engine/RHI/DX12/DX12Device.h"
#include "Engine/RHI/DX12/DX12SwapChain.h"
#include "Engine/RHI/DX12/DX12CommandContext.h"
#include "Engine/RHI/DX12/DX12GpuProfiler.h"
#include "Panels/DockspacePanel.h"
#include "Panels/MenuBar.h"
#include "Panels/CVarPanel.h"
#include "Panels/LogPanel.h"
#include "Panels/StatsPanel.h"
#include "TriangleVertex.h"

#include <directx/d3dx12.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <format>
#include <fstream>
#include <stdexcept>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

using Microsoft::WRL::ComPtr;

namespace
{
	auto& r_clearColorR = CVarRegistry::RegisterFloat("r.clearColor.r", 0.0f, 0.0f, 1.0f);
	auto& r_clearColorG = CVarRegistry::RegisterFloat("r.clearColor.g", 0.2f, 0.0f, 1.0f);
	auto& r_clearColorB = CVarRegistry::RegisterFloat("r.clearColor.b", 0.4f, 0.0f, 1.0f);
}

namespace Editor
{
	static void ThrowIfFailed(HRESULT hr, const char* msg)
	{
		if (FAILED(hr))
		{
			ROYAL_LOG_FATAL("{}, hr={:#x}", msg, static_cast<unsigned>(hr));
			throw std::runtime_error(msg);
		}
	}

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
		// Register sink with Logger that forwards into LogPanel.
		Logger::AddSink([](LogLevel level, const std::string& message)
			{
				Panels::LogPanel::Get().AddLog(level, message);
			});

		ROYAL_LOG_INFO("Editor initializing...");

		m_device = std::make_unique<RHI::DX12Device>();
		m_swapChain = std::make_unique<RHI::DX12SwapChain>(
			*m_device, GetWindow().GetHandle(), GetWindow().GetWidth(), GetWindow().GetHeight()
		);
		m_commandContext = std::make_unique<RHI::DX12CommandContext>(*m_device);

		m_gpuProfiler = std::make_unique<RHI::DX12GpuProfiler>(*m_device);

		GetWindow().SetResizeCallback([this](uint32_t width, uint32_t height) { OnWindowResize(width, height); });

		CreateRootSignature();
		CreatePipelineState();
		CreateVertexBuffer();

		InitImGui();

		ROYAL_LOG_INFO("Editor initialized.");
	}

	void EditorApp::OnUpdate(float deltaTime)
	{
		
	}

	void EditorApp::OnRender()
	{
		// Begin profiler frames.
		Profiler::Get().BeginFrame();

		m_commandContext->Reset();
		ID3D12GraphicsCommandList* commandList = m_commandContext->GetCommandList();

		m_gpuProfiler->BeginFrame(commandList);
		uint32_t frameEvent = m_gpuProfiler->BeginEvent(commandList, "Frame");

		ID3D12Resource* backBuffer = nullptr;
		{
			ROYAL_PROFILE_SCOPE("Scene Draw");

			// Set necessary state.
			commandList->SetGraphicsRootSignature(m_rootSignature.Get());
			commandList->SetPipelineState(m_pipelineState.Get());

			D3D12_VIEWPORT viewport{ 0.0f, 0.0f,
			static_cast<float>(GetWindow().GetWidth()), static_cast<float>(GetWindow().GetHeight()), 0.0f, 1.0f };
			D3D12_RECT scissor{ 0, 0, static_cast<LONG>(GetWindow().GetWidth()), static_cast<LONG>(GetWindow().GetHeight()) };
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissor);

			backBuffer = m_swapChain->GetCurrentBackBuffer();

			// Indicate that the back buffer will be used as a render target.
			D3D12_RESOURCE_BARRIER toRT = CD3DX12_RESOURCE_BARRIER::Transition(
				backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &toRT);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain->GetCurrentRTV();
			commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

			// Record commands.
			const float clearColor[] = { r_clearColorR.Get(), r_clearColorG.Get(), r_clearColorB.Get(), 1.0f };
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
			commandList->DrawInstanced(3, 1, 0, 0);
		}

		{
			ROYAL_PROFILE_SCOPE("ImGui Draw");

			// Build UI for new frame.
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			Panels::DrawDockspace();
			Panels::DrawMenuBar(m_panelState, *this);
			Panels::DrawStatsPanel(m_panelState.showStats, GetWindow(), m_gpuProfiler->GetLastFrameSamples());
			Panels::LogPanel::Get().Draw(m_panelState.showLog);
			Panels::DrawCVarPanel(m_panelState.showCVars);
			//ImGui::ShowDemoWindow();

			ImGui::Render();

			// Render UI as an overlay after the scene draws.
			ID3D12DescriptorHeap* imGuiHeaps[] = { m_imGuiSrvHeap.Get() };
			commandList->SetDescriptorHeaps(1, imGuiHeaps);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
		}

		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &toPresent);

		m_gpuProfiler->EndEvent(commandList, frameEvent);
		m_gpuProfiler->EndFrame(commandList);

		{
			ROYAL_PROFILE_SCOPE("GPU Wait");
			m_commandContext->ExecuteAndWait();
		}
		m_gpuProfiler->Readback();
		m_swapChain->Present();
	}

	void EditorApp::OnShutdown()
	{
		ShutdownImGui();
		m_commandContext.reset();
		m_swapChain.reset();
		m_device.reset();
	}

	void EditorApp::InitImGui()
	{
		// Describe and create a shader resoure view (SRV) descriptor heap for font/texture data.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = 64;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		ThrowIfFailed(m_device->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_imGuiSrvHeap)), "Failed to create ImGui SRV heap.");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplWin32_Init(GetWindow().GetHandle());

		ImGui_ImplDX12_InitInfo initInfo{};
		initInfo.Device = m_device->GetDevice();
		initInfo.CommandQueue = m_device->GetCommandQueue();
		initInfo.NumFramesInFlight = RHI::DX12SwapChain::kFrameCount;
		initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
		initInfo.SrvDescriptorHeap = m_imGuiSrvHeap.Get();

		// Free-list-style allocator for SRV descriptors, required for ImGui DX12 backend.
		initInfo.SrvDescriptorAllocFn =
			[](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
			{
				auto* self = static_cast<EditorApp*>(info->UserData);
				self->AllocateImGuiSrvDescriptor(outCpu, outGpu);
			};
		initInfo.SrvDescriptorFreeFn =
			[](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu)
			{
				// No-op for now.
			};
		initInfo.UserData = this;

		ImGui_ImplDX12_Init(&initInfo);

		// Forward Win32 messages to ImGui's Win32 backend.
		GetWindow().SetMessageHook(
			[](HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam);
			}
		);
	}

	void EditorApp::ShutdownImGui()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorApp::AllocateImGuiSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
	{
		if (m_imGuiSrvDescriptorSize == 0)
		{
			m_imGuiSrvDescriptorSize = m_device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		uint32_t index = m_imGuiNextSrvIndex++;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_imGuiSrvHeap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += static_cast<SIZE_T>(index) * m_imGuiSrvDescriptorSize;

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_imGuiSrvHeap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += static_cast<UINT64>(index) * m_imGuiSrvDescriptorSize;

		*outCpu = cpuHandle;
		*outGpu = gpuHandle;
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

		ThrowIfFailed(m_device->GetDevice()->CreateRootSignature(0,
			signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)
		), "Failed to create root signature.");
	}

	void EditorApp::CreatePipelineState()
	{
		ROYAL_ASSERT(m_rootSignature != nullptr, "CreatePipelineState called before CreateRootDescriptor");

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

		ThrowIfFailed(m_device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)), "Failed to create pipeline state.");
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

		ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)
		), "Failed to create vertex buffer.");

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
}