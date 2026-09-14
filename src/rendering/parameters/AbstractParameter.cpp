#include "AbstractParameter.h"

namespace sahara::rendering
{

AbstractParameter::AbstractParameter(const QString& name)
	: m_name(name)
	, m_visible(true)
{
}

const QString& AbstractParameter::name() const
{
	return m_name;
}

bool AbstractParameter::isVisible() const noexcept
{
	return m_visible;
}

void AbstractParameter::setVisible(bool visible)
{
	if (m_visible != visible)
	{
		m_visible = visible;
		emit visibilityChanged(visible);
	}
}

}
