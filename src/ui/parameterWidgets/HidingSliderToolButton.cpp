#include "HidingSliderToolButton.h"

namespace sahara::ui
{

HidingSliderToolButton::HidingSliderToolButton(int minimum, int maximum, int stepsize, int value, QWidget* parent)
	: QToolButton(parent)
	, m_slider(minimum, maximum, stepsize, value)
{
	setAutoRaise(true);
	setIcon(QIcon(":/icons/parameter-slider.png"));
	setStyleSheet("QToolButton {background-color: transparent;}");
	setToolTip("Use slider");

	connect(this, &QToolButton::clicked, this, &HidingSliderToolButton::showSlider);
	connect(&m_slider, &QSlider::valueChanged, this, &HidingSliderToolButton::valueChanged);
}

void HidingSliderToolButton::setValue(int value)
{
	m_slider.setValue(value);
}

void HidingSliderToolButton::showSlider()
{
	m_slider.move(QCursor::pos());
	m_slider.setFocus();
	m_slider.show();
}

}