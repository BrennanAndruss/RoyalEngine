#include "Engine/Core/CVar.h"

namespace Royal
{
	std::vector<std::unique_ptr<CVarBase>>& CVarRegistry::Storage()
	{
		static std::vector<std::unique_ptr<CVarBase>> s_cvars;
		return s_cvars;
	}

	const std::vector<std::unique_ptr<CVarBase>>& CVarRegistry::GetAll()
	{
		return Storage();
	}

	CVarBool& CVarRegistry::RegisterBool(const std::string& name, bool defaultValue)
	{
		auto cvar = std::make_unique<CVarBool>(name, defaultValue, false, true);
		CVarBool& ref = *cvar;
		Storage().push_back(std::move(cvar));
		return ref;
	}

	CVarInt& CVarRegistry::RegisterInt(const std::string& name, int defaultValue, int min, int max)
	{
		auto cvar = std::make_unique<CVarInt>(name, defaultValue, min, max);
		CVarInt& ref = *cvar;
		Storage().push_back(std::move(cvar));
		return ref;
	}

	CVarFloat& CVarRegistry::RegisterFloat(const std::string& name, float defaultValue, float min, float max)
	{
		auto cvar = std::make_unique<CVarFloat>(name, defaultValue, min, max);
		CVarFloat& ref = *cvar;
		Storage().push_back(std::move(cvar));
		return ref;
	}
}