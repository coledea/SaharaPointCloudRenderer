#include "TriggerParameter.h"

namespace sahara::rendering
{

TriggerParameter::TriggerParameter(const QString& name)
	: AbstractParameter(name)
	, m_active(false)
{
}

void TriggerParameter::activate() noexcept
{
	m_active = true;
	emit valueChanged();
}

void TriggerParameter::deactivate() noexcept
{
	m_active = false;
	emit valueChanged();
}

void TriggerParameter::toggle() noexcept
{
	m_active = !m_active;
	emit valueChanged();
}

bool TriggerParameter::isActive() const noexcept
{
	return m_active;
}

void TriggerParameter::reset()
{
	deactivate();
}

}