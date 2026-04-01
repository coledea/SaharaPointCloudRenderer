#include "EnumParameterWidget.h"

namespace sahara::ui
{

EnumParameterWidget::EnumParameterWidget(rendering::EnumParameter* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-enum.png",
		  parent)
	, m_parameter(parameter)
{
	for (const auto& entry : parameter->entries())
	{
		m_combo_box.addItem(entry);
	}

	m_layout.insertWidget(1, &m_combo_box); // insert after icon
	m_combo_box.setCurrentIndex(m_parameter->value());
	connect(&m_combo_box, &QComboBox::currentIndexChanged, m_parameter, &rendering::EnumParameter::setValue);
}

void EnumParameterWidget::onParameterValueChanged()
{
	auto new_value = m_parameter->value();
	if (m_combo_box.currentIndex() != new_value)
	{
		const QSignalBlocker blocker(&m_combo_box);
		m_combo_box.setCurrentIndex(new_value);
	}
}

}