#pragma once

#include "ParameterWidget.h"
#include "SliderAndSpinBoxFloat.h"

namespace sahara::ui
{

class Vector3DParameterWidget : public ParameterWidget
{
	Q_OBJECT

public:
	Vector3DParameterWidget(rendering::RangeParameter<QVector3D>* parameter, QWidget* parent = nullptr);

private:
	rendering::RangeParameter<QVector3D>* m_parameter;
	std::array<SliderAndSpinBoxFloat*, 3> m_slider_and_spinbox;
	QToolButton m_linking_button;

	void updateParameter(float new_value, int changed_component);
	void onParameterValueChanged() override;
};

}