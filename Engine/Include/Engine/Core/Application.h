#pragma once

#include <string>

namespace Royal
{
	struct ApplicationConfig
	{
		std::string name = "Royal Engine";
	};

	class Application
	{
	public:
		explicit Application(const ApplicationConfig& config);
		virtual ~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void Run();

	protected:
		virtual void OnInit() {}

	private:
		ApplicationConfig m_config;
	};
}