#include "AbstractParameter.h"

namespace sahara::rendering
{

AbstractParameter::AbstractParameter(const QString& name)
	: m_name(name)
{
}

const QString& AbstractParameter::name() const
{
	return m_name;
}

}