#pragma once

#include "HidingSliderToolButton.h"
#include "rendering/parameters/RangeParameter.h"

#include <QSpinBox>

namespace sahara::ui
{

class SliderAndSpinBoxFloat : public QWidget
{
	Q_OBJECT

public:
	SliderAndSpinBoxFloat(float minimum, float maximum, float stepsize, float value, QWidget* parent = nullptr);

	float value() const;
	void setValue(float value);

signals:
	void valueChanged(float value);

private:
	const float DECIMAL_PRECISION = 10000.0f;
	QDoubleSpinBox* m_spinbox;
	HidingSliderToolButton* m_slider;
};

}