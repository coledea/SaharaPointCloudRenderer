#include "Vector3DParameterWidget.h"

#include <QGridLayout>

namespace sahara::ui
{

Vector3DParameterWidget::Vector3DParameterWidget(rendering::RangeParameter<QVector3D>* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-float-vec3.png",
		  parent)
	, m_parameter(parameter)
	, m_slider_and_spinbox{ new SliderAndSpinBoxFloat(parameter->minimum().x(), parameter->maximum().x(), parameter->stepsize().x(), parameter->value().x(), this),
							new SliderAndSpinBoxFloat(parameter->minimum().y(), parameter->maximum().y(), parameter->stepsize().y(), parameter->value().y(), this),
							new SliderAndSpinBoxFloat(parameter->minimum().z(), parameter->maximum().z(), parameter->stepsize().z(), parameter->value().z(), this) }
{
	auto widget = new QWidget();
	auto layout = new QGridLayout();

	m_linking_button.setIcon(QIcon(":/icons/general-linked.png"));
	m_linking_button.setCheckable(true);
	m_linking_button.setChecked(false);
	m_linking_button.setAutoRaise(true);
	m_linking_button.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	m_linking_button.setIconSize(QSize(16, 32));
	m_linking_button.setAutoFillBackground(false);
	m_linking_button.setToolTip("Toggle joint manipulation of components");

	layout->addWidget(&m_linking_button, 0, 2, 3, 1);
	layout->addWidget(m_slider_and_spinbox[0], 0, 0, 1, 2);
	layout->addWidget(m_slider_and_spinbox[1], 1, 0, 1, 2);
	layout->addWidget(m_slider_and_spinbox[2], 2, 0, 1, 2);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	widget->setLayout(layout);

	m_layout.insertWidget(1, widget); // insert after icon

	connect(m_slider_and_spinbox[0], &SliderAndSpinBoxFloat::valueChanged, this, [&](float value) { updateParameter(value, 0); });
	connect(m_slider_and_spinbox[1], &SliderAndSpinBoxFloat::valueChanged, this, [&](float value) { updateParameter(value, 1); });
	connect(m_slider_and_spinbox[2], &SliderAndSpinBoxFloat::valueChanged, this, [&](float value) { updateParameter(value, 2); });
}

void Vector3DParameterWidget::updateParameter(float new_value, int changed_component)
{
	if (m_linking_button.isChecked())
	{
		m_parameter->setValue(QVector3D(new_value, new_value, new_value));
	}
	else
	{
		auto changed_parameter_value = m_parameter->value();
		changed_parameter_value[changed_component] = new_value;
		m_parameter->setValue(changed_parameter_value);
	}
}

void Vector3DParameterWidget::onParameterValueChanged()
{
	auto new_value = m_parameter->value();

	for (int i = 0; i < 3; i++)
	{
		if (m_slider_and_spinbox[i]->value() != new_value[i])
		{
			const QSignalBlocker blocker(m_slider_and_spinbox[i]);
			m_slider_and_spinbox[i]->setValue(new_value[i]);
		}
	}
}

}