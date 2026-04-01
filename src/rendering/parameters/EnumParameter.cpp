#include "EnumParameter.h"

namespace sahara::rendering
{

EnumParameter::EnumParameter(const QString& name, const std::vector<QString>& entries, uint initial_index)
	: Parameter<uint>(name, initial_index)
	, m_entries(entries)
{
}

EnumParameter::EnumParameter(const QString& name, std::initializer_list<std::string> entries, uint initial_index)
	: Parameter<uint>(name, initial_index)
{
	for (const auto& entry : entries)
	{
		m_entries.push_back(entry.c_str());
	}
}

const std::vector<QString>& EnumParameter::entries()
{
	return m_entries;
}

const QString& EnumParameter::selectedEntry() const
{
	return m_entries[value()];
}

}