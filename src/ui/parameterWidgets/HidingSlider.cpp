#include "HidingSlider.h"

namespace sahara::ui
{

HidingSlider::HidingSlider(int minimum, int maximum, int stepsize, int value, QWidget* parent)
	: QSlider(parent)
{
	setMinimum(minimum);
	setMaximum(maximum);
	setSingleStep(stepsize);
	setValue(value);
	setOrientation(Qt::Vertical);
	setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::MSWindowsFixedSizeDialogHint);
}

void HidingSlider::focusOutEvent(QFocusEvent* event)
{
	QSlider::focusOutEvent(event);
	hide();
}

void HidingSlider::leaveEvent(QEvent* event)
{
	QSlider::leaveEvent(event);
	hide();
}

}