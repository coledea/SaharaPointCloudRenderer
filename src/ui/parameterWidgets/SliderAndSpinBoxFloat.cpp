#include "SliderAndSpinBoxFloat.h"

#include <QHBoxLayout>

namespace sahara::ui
{

SliderAndSpinBoxFloat::SliderAndSpinBoxFloat(float minimum, float maximum, float stepsize, float value, QWidget* parent)
	: QWidget(parent)
	, m_spinbox(new QDoubleSpinBox(this))
{
	m_spinbox->setDecimals(4);
	m_spinbox->setMinimum(minimum);
	m_spinbox->setMaximum(maximum);
	m_spinbox->setSingleStep(stepsize);
	m_spinbox->setValue(value);

	m_slider = new HidingSliderToolButton(
		static_cast<int>(minimum * DECIMAL_PRECISION),
		static_cast<int>(maximum * DECIMAL_PRECISION),
		static_cast<int>(stepsize * DECIMAL_PRECISION),
		static_cast<int>(value * DECIMAL_PRECISION),
		this);

	auto layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	layout->addWidget(m_spinbox);
	layout->addWidget(m_slider);
	setLayout(layout);

	connect(m_spinbox, &QDoubleSpinBox::valueChanged, this, [&](double value) {
		const QSignalBlocker blocker(m_slider);
		float valueF = static_cast<float>(value);
		m_slider->setValue(static_cast<int>(valueF * DECIMAL_PRECISION));
		emit valueChanged(valueF);
	});

	connect(m_slider, &HidingSliderToolButton::valueChanged, this, [&](int value) {
		const QSignalBlocker blocker(m_spinbox);
		m_spinbox->setValue(static_cast<float>(value) / DECIMAL_PRECISION);
		emit valueChanged(static_cast<float>(value) / DECIMAL_PRECISION);
	});
}

float SliderAndSpinBoxFloat::value() const
{
	return static_cast<float>(m_spinbox->value());
}

void SliderAndSpinBoxFloat::setValue(float value)
{
	m_spinbox->setValue(value);
}

}