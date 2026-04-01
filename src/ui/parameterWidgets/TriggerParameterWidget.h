#pragma once

#include "ParameterWidget.h"
#include "rendering/parameters/TriggerParameter.h"

#include <QPushButton>

namespace sahara::ui
{

class TriggerParameterWidget : public ParameterWidget
{
public:
	TriggerParameterWidget(rendering::TriggerParameter* parameter, QWidget* parent = nullptr);

private:
	QPushButton m_button;
	rendering::TriggerParameter* m_parameter;

	void onParameterValueChanged() override;
};

}