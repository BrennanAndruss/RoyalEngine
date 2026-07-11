#include "Engine/Platform/Window.h"

#include <stdexcept>

namespace Royal
{
	static std::wstring ToWide(const std::string& str)
	{
		if (str.empty()) return {};
		int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		std::wstring result(size, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), result.data(), size);
		return result;
	}

	Window::Window(const WindowConfig& config)
		: m_width(config.width)
		, m_height(config.height)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		// INitialize the window class.
		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = hInstance;
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.lpszClassName = kWindowClassName;

		if (!RegisterClassEx(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			throw std::runtime_error("Failed to register window class.");
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		if (!config.resizable)
		{
			style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		}

		RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
		AdjustWindowRect(&windowRect, style, FALSE);

		std::wstring wideTitle = ToWide(config.title);

		// Create the window and store a handle to it.
		m_hwnd = CreateWindowEx(
			0,
			kWindowClassName,
			wideTitle.c_str(),
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,
			nullptr,
			hInstance,
			this
		);

		if (!m_hwnd)
		{
			throw std::runtime_error("Failed to create window.");
		}

		ShowWindow(m_hwnd, SW_SHOW);
	}

	Window::~Window()
	{
		if (m_hwnd)
		{
			DestroyWindow(m_hwnd);
		}
	}

	void Window::PollEvents()
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* window = nullptr;

		if (message == WM_NCCREATE)
		{
			LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			window = static_cast<Window*>(createStruct->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		else
		{
			window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (window)
		{
			return window->HandleMessage(hwnd, message, wParam, lParam);
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	LRESULT Window::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_messageHook)
		{
			m_messageHook(hwnd, message, wParam, lParam);
		}

		switch (message)
		{
		case WM_CLOSE:
		case WM_DESTROY:
			m_shouldClose = true;
			return 0;

		case WM_SIZE: 
		{
			uint32_t newWidth = LOWORD(lParam);
			uint32_t newHeight = HIWORD(lParam);
			if (newWidth != 0 && newHeight != 0 && (newWidth != m_width || newHeight != m_height))
			{
				m_width = newWidth;
				m_height = newHeight;
				if (m_onResize)
				{
					m_onResize(m_width, m_height);
				}
			}
			return 0;
		}

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
}