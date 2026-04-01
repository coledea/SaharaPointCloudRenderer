#pragma once

#include "ParameterWidget.h"
#include "SliderAndSpinBoxFloat.h"
#include "rendering/parameters/RangeParameter.h"

namespace sahara::ui
{

class FloatParameterWidget : public ParameterWidget
{
public:
	FloatParameterWidget(rendering::RangeParameter<float>* parameter, QWidget* parent = nullptr);

private:
	static constexpr float DECIMAL_PRECISION = 100.0;
	SliderAndSpinBoxFloat m_slider_and_spinbox;
	rendering::RangeParameter<float>* m_parameter;

	void onParameterValueChanged() override;
};

}