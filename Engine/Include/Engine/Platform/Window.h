#pragma once

#include "Engine/Platform/WindowConfig.h"

#include <Windows.h>
#include <cstdint>
#include <functional>

namespace Royal
{
	class Window
	{
	public:
		explicit Window(const WindowConfig& config);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		void PollEvents();

		bool ShouldClose() const { return m_shouldClose; }
		HWND GetHandle() const { return m_hwnd; }
		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }

		using ResizeCallback = std::function<void(uint32_t, uint32_t)>;
		void SetResizeCallback(ResizeCallback callback) { m_onResize = std::move(callback); }

	private:
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

		HWND m_hwnd = nullptr;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		bool m_shouldClose = false;
		ResizeCallback m_onResize;

		static constexpr const wchar_t* kWindowClassName = L"RoyalEngineWindowClass";
	};
}