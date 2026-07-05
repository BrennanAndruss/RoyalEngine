#include "Engine/Core/Application.h"

namespace Royal
{
	Application::Application(const ApplicationConfig& config)
		: m_config(config)
	{

	}

	Application::~Application() = default;

	void Application::Run()
	{
		OnInit();
	}
}