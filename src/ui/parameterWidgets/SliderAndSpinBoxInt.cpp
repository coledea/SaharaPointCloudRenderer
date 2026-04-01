#include "SliderAndSpinBoxInt.h"

#include <QHBoxLayout>

namespace sahara::ui
{

SliderAndSpinBoxInt::SliderAndSpinBoxInt(int minimum, int maximum, int stepsize, int value, QWidget* parent)
	: QWidget(parent)
	, m_spinbox(new QSpinBox(this))
{
	// we want the spin box to match the size of the double spin boxes that have 4 decimal places
	m_spinbox->setMaximum(10000);
	m_spinbox->updateGeometry();
	int minimum_width = m_spinbox->minimumSizeHint().width();

	m_spinbox->setMinimum(minimum);
	m_spinbox->setMaximum(maximum);
	m_spinbox->setSingleStep(stepsize);
	m_spinbox->setValue(value);
	m_spinbox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_spinbox->setMinimumWidth(minimum_width);

	m_slider = new HidingSliderToolButton(minimum, maximum, stepsize, value, this);

	auto layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(5);

	layout->addWidget(m_spinbox);
	layout->addWidget(m_slider);

	setLayout(layout);

	connect(dynamic_cast<QSpinBox*>(m_spinbox), &QSpinBox::valueChanged, this, [&](int value) {
		const QSignalBlocker blocker(m_slider);
		m_slider->setValue(value);
		emit valueChanged(value);
	});

	connect(m_slider, &HidingSliderToolButton::valueChanged, this, [&](int value) {
		const QSignalBlocker blocker(m_spinbox);
		dynamic_cast<QSpinBox*>(m_spinbox)->setValue(value);
		emit valueChanged(value);
	});
}

int SliderAndSpinBoxInt::value() const
{
	return m_spinbox->value();
}

void SliderAndSpinBoxInt::setValue(int value)
{
	m_slider->setValue(value);
}

}
