#pragma once

#include <cstdint>
#include <string>

namespace Royal
{
	struct WindowConfig
	{
		uint32_t width = 1280;
		uint32_t height = 720;
		std::string title = "Royal Engine";
		bool resizable = true;
	};
}