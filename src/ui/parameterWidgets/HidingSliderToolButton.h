#pragma once

#include "HidingSlider.h"

#include <QToolButton>

namespace sahara::ui
{

class HidingSliderToolButton : public QToolButton
{
	Q_OBJECT

public:
	HidingSliderToolButton(int minimum, int maximum, int stepsize, int value, QWidget* parent = nullptr);

	void setValue(int value);

signals:
	void valueChanged(int value);

private:
	HidingSlider m_slider;

	void showSlider();
};

}