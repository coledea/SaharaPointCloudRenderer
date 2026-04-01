#pragma once

#include "Parameter.h"

namespace sahara::rendering
{

class EnumParameter : public Parameter<uint>
{
public:
	EnumParameter(const QString& name, const std::vector<QString>& entries, uint initial_index);
	EnumParameter(const QString& name, std::initializer_list<std::string> entries, uint initial_index); // for less-verbose in-place specification of the enum keys.
	const std::vector<QString>& entries();
	const QString& selectedEntry() const;

private:
	std::vector<QString> m_entries;
};

}