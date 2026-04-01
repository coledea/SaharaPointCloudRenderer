#include "IntParameterWidget.h"

namespace sahara::ui
{

IntParameterWidget::IntParameterWidget(rendering::RangeParameter<int>* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-float.png",
		  parent)
	, m_slider_and_spinbox(
		  parameter->minimum(),
		  parameter->maximum(),
		  parameter->stepsize(),
		  parameter->value(),
		  this)
	, m_parameter(parameter)
{
	m_layout.insertWidget(1, &m_slider_and_spinbox); // insert after icon
	connect(&m_slider_and_spinbox, &SliderAndSpinBoxInt::valueChanged, m_parameter, &rendering::RangeParameter<int>::setValue);
}

void IntParameterWidget::onParameterValueChanged()
{
	auto new_value = m_parameter->value();
	if (m_slider_and_spinbox.value() != new_value)
	{
		const QSignalBlocker blocker(&m_slider_and_spinbox);
		m_slider_and_spinbox.setValue(new_value);
	}
}

}