#pragma once

#include "ParameterWidget.h"
#include "rendering/parameters/EnumParameter.h"

#include <QComboBox>

namespace sahara::ui
{

class EnumParameterWidget : public ParameterWidget
{
public:
	EnumParameterWidget(rendering::EnumParameter* parameter, QWidget* parent = nullptr);

private:
	QComboBox m_combo_box;
	rendering::EnumParameter* m_parameter;

	void onParameterValueChanged() override;
};

}