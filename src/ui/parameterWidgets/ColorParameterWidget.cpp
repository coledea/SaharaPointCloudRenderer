#include "ColorParameterWidget.h"

namespace sahara::ui
{

ColorParameterWidget::ColorParameterWidget(rendering::Parameter<QColor>* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-color.png",
		  parent)
	, m_parameter(parameter)
	, m_choose_color_button("Choose", this)
	, m_color_dialog(parameter->value(), this)
{
	m_color_dialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);

	m_choose_color_button.setStyleSheet("background-color:" + parameter->value().name());
	m_layout.insertWidget(1, &m_choose_color_button); // insert after icon

	connect(&m_choose_color_button, &QPushButton::clicked, &m_color_dialog, &QColorDialog::show);
	connect(&m_color_dialog, &QColorDialog::currentColorChanged, this, &ColorParameterWidget::onColorSelected);
}

void ColorParameterWidget::onParameterValueChanged()
{
	auto newColor = m_parameter->value();
	if (m_color_dialog.currentColor() != newColor)
	{
		const QSignalBlocker blocker(&m_color_dialog);
		m_color_dialog.setCurrentColor(newColor);
		m_choose_color_button.setStyleSheet("background-color:" + newColor.name());
	}
}

void ColorParameterWidget::onColorSelected(const QColor& color)
{
	m_parameter->setValue(color);
	m_choose_color_button.setStyleSheet("background-color:" + color.name());
}

}