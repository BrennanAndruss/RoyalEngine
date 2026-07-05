#include "Engine/Core/Application.h"

#include "Engine/Platform/Window.h"

namespace Royal
{
	Application::Application(const ApplicationConfig& config)
		: m_config(config)
		, m_window(std::make_unique<Window>(config.window))
	{

	}

	Application::~Application() = default;

	void Application::Run()
	{
		OnInit();
		m_running = true;

		while (m_running && !m_window->ShouldClose())
		{
			m_window->PollEvents();

			float deltaTime = 0.0f;
			OnUpdate(deltaTime);
			OnRender();
		}

		OnShutdown();
	}
}