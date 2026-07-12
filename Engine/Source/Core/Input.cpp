#include "Engine/Core/Input.h"

namespace Royal
{
	std::array<bool, 256> Input::s_keysDown{};
	std::array<bool, 256> Input::s_prevKeysDown{};
}