#include "BoolParameterWidget.h"

namespace sahara::ui
{

BoolParameterWidget::BoolParameterWidget(rendering::Parameter<bool>* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-bool.png",
		  parent)
	, m_parameter(parameter)
{
	m_checkbox.setChecked(parameter->value());
	m_layout.insertWidget(1, &m_checkbox); // insert after icon
	connect(&m_checkbox, &QCheckBox::toggled, m_parameter, &rendering::Parameter<bool>::setValue);
}

void BoolParameterWidget::onParameterValueChanged()
{
	auto new_value = m_parameter->value();
	if (m_checkbox.isChecked() != new_value)
	{
		const QSignalBlocker blocker(&m_checkbox);
		m_checkbox.setChecked(new_value);
	}
}

}