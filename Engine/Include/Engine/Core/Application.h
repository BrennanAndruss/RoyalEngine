#pragma once

#include "Engine/Platform/WindowConfig.h"

#include <memory>
#include <string>

namespace Royal
{
	class Window;

	struct ApplicationConfig
	{
		std::string name = "Royal Engine";
		WindowConfig window;
	};

	class Application
	{
	public:
		explicit Application(const ApplicationConfig& config);
		virtual ~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void Run();
		void RequestClose() { m_running = false; }

	protected:
		virtual void OnInit() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnShutdown() {}

		Window& GetWindow() { return *m_window; }

	private:
		ApplicationConfig m_config;
		std::unique_ptr<Window> m_window;
		bool m_running = false;
	};
}