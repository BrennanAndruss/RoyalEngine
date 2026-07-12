#pragma once

#include <array>
#include <cstdint>

namespace Royal
{
	class Input
	{
	public:
		static bool IsKeyDown(uint8_t vkCode) { return s_keysDown[vkCode]; }
		static bool WasKeyPressed(uint8_t vkCode)
		{
			return s_keysDown[vkCode] && !s_prevKeysDown[vkCode];
		}

		static void OnKeyDown(uint8_t vkCode) { s_keysDown[vkCode] = true; }
		static void OnKeyUp(uint8_t vkCode) { s_keysDown[vkCode] = false; }
		static void EndFrame() { s_prevKeysDown = s_keysDown; }

	private:
		static std::array<bool, 256> s_keysDown;
		static std::array<bool, 256> s_prevKeysDown;
	};
}