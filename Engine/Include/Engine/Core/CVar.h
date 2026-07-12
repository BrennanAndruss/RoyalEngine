#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Royal
{
	enum class CVarType
	{
		Bool,
		Int,
		Float,
	};

	// Type-erased base so the registry can hold heterogenous CVars in one list.
	class CVarBase
	{
	public:
		CVarBase(std::string name, CVarType type) : m_name(std::move(name)), m_type(type) {}
		virtual ~CVarBase() = default;

		const std::string& GetName() const { return m_name; }
		CVarType GetType() const { return m_type; }

	private:
		std::string m_name;
		CVarType m_type;
	};

	template<typename T, CVarType Type>
	class CVarT : public CVarBase
	{
	public:
		CVarT(std::string name, T defaultValue, T min, T max)
			: CVarBase(std::move(name), Type), m_value(defaultValue), m_min(min), m_max(max) {}

		T Get() const { return m_value; }
		void Set(T value) { m_value = value; }

		T GetMin() const { return m_min; }
		T GetMax() const { return m_max; }

		// Direct pointer access, to bind straight to the value.
		T* GetValuePtr() { return &m_value; }

	private:
		T m_value;
		T m_min;
		T m_max;
	};

	using CVarBool = CVarT<bool, CVarType::Bool>;
	using CVarInt = CVarT<int, CVarType::Int>;
	using CVarFloat = CVarT<float, CVarType::Float>;

	class CVarRegistry
	{
	public:
		static const std::vector<std::unique_ptr<CVarBase>>& GetAll();

		static CVarBool& RegisterBool(const std::string& name, bool defaultValue);
		static CVarInt& RegisterInt(const std::string& name, int defaultValue, int min, int max);
		static CVarFloat& RegisterFloat(const std::string& name, float defaultValue, float min, float max);

	private:
		static std::vector<std::unique_ptr<CVarBase>>& Storage();
	};
}