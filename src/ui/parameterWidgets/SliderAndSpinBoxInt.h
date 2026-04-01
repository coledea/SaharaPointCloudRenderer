#pragma once

#include "HidingSliderToolButton.h"

#include <QSpinBox>

namespace sahara::ui
{

class SliderAndSpinBoxInt : public QWidget
{
	Q_OBJECT

public:
	SliderAndSpinBoxInt(int minimum, int maximum, int stepsize, int value, QWidget* parent = nullptr);

	int value() const;
	void setValue(int value);

signals:
	void valueChanged(int value);

private:
	QSpinBox* m_spinbox;
	HidingSliderToolButton* m_slider;
};

}