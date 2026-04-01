#pragma once

#include "ParameterWidget.h"
#include "SliderAndSpinBoxInt.h"
#include "rendering/parameters/RangeParameter.h"

namespace sahara::ui
{

class IntParameterWidget : public ParameterWidget
{
public:
	IntParameterWidget(rendering::RangeParameter<int>* parameter, QWidget* parent = nullptr);

private:
	SliderAndSpinBoxInt m_slider_and_spinbox;
	rendering::RangeParameter<int>* m_parameter;

	void onParameterValueChanged() override;
};

}