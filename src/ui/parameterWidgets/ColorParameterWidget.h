#pragma once

#include "ParameterWidget.h"
#include "rendering/parameters/RangeParameter.h"

#include <QColorDialog>
#include <QPushButton>

namespace sahara::ui
{

class ColorParameterWidget : public ParameterWidget
{
public:
	ColorParameterWidget(rendering::Parameter<QColor>* parameter, QWidget* parent = nullptr);

private:
	QPushButton m_choose_color_button;
	QColorDialog m_color_dialog;
	rendering::Parameter<QColor>* m_parameter;

	void onParameterValueChanged() override;
	void onColorSelected(const QColor& color);
};

}