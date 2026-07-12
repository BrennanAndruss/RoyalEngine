#include "Engine/Core/Application.h"

#include "Engine/Core/Input.h"
#include "Engine/Core/Time.h"
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
		Time::Init();
		m_running = true;

		while (m_running && !m_window->ShouldClose())
		{
			m_window->PollEvents();
			Time::Tick();

			OnUpdate(Time::GetDeltaTime());
			OnRender();
			Input::EndFrame();
		}

		OnShutdown();
	}
}