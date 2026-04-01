#pragma once

#include "ParameterWidget.h"
#include "rendering/parameters/RangeParameter.h"

#include <QCheckBox>

namespace sahara::ui
{

class BoolParameterWidget : public ParameterWidget
{
public:
	BoolParameterWidget(rendering::Parameter<bool>* parameter, QWidget* parent = nullptr);

private:
	QCheckBox m_checkbox;
	rendering::Parameter<bool>* m_parameter;

	void onParameterValueChanged() override;
};

}