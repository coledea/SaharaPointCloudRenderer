#include "TriggerParameterWidget.h"

namespace sahara::ui
{

TriggerParameterWidget::TriggerParameterWidget(rendering::TriggerParameter* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-bool.png",
		  parent)
	, m_parameter(parameter)
	, m_button(parameter->name(), this)
{
	m_layout.insertWidget(1, &m_button); // insert after icon
	connect(&m_button, &QPushButton::clicked, m_parameter, &rendering::TriggerParameter::toggle);
}

void TriggerParameterWidget::onParameterValueChanged()
{
	auto is_active = m_parameter->isActive();
	if (m_button.isChecked() != is_active)
	{
		const QSignalBlocker blocker(&m_button);
		m_button.setChecked(is_active);
	}
}

}