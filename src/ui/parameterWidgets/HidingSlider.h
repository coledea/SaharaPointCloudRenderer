#pragma once

#include <QSlider>

namespace sahara::ui
{

class HidingSlider : public QSlider
{
	Q_OBJECT

public:
	HidingSlider(int minimum, int maximum, int stepsize, int value, QWidget* parent = nullptr);

private:
	void focusOutEvent(QFocusEvent* event) override;
	void leaveEvent(QEvent* event) override;
};

}