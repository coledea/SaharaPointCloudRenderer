#include "EasingCurveIcons.h"

#include <QIcon>
#include <QMetaEnum>
#include <QPainter>
#include <QPainterPath>

namespace sahara::utils
{

// adapted from the official QT example: https://doc.qt.io/qt-6/qtwidgets-animation-easing-example.html
std::array<QIcon, QEasingCurve::NCurveTypes - 3> drawIcons()
{
	std::array<QIcon, QEasingCurve::NCurveTypes - 3> icons;

	const QSize icon_size(32, 32);
	QPixmap pixmap(icon_size);
	QPainter painter(&pixmap);

	QLinearGradient gradient(0, 0, 0, icon_size.height());
	gradient.setColorAt(0.0, QColor(240, 240, 240));
	gradient.setColorAt(1.0, QColor(224, 224, 224));
	QBrush brush(gradient);

	const float curve_scale = icon_size.height() / 2;
	const int x_axis_position = static_cast<float>(icon_size.height()) / 1.5;
	const int y_axis_position = static_cast<float>(icon_size.width()) / 3;

	for (size_t i = 0; i < QEasingCurve::NCurveTypes - 3; i++)
	{ // excludes the last three custom curve types
		QEasingCurve curve(static_cast<QEasingCurve::Type>(i));

		// background
		painter.fillRect(QRect(QPoint(0, 0), icon_size), brush);

		// axes
		painter.setPen(QColor(0, 0, 255, 64));
		painter.drawLine(0, x_axis_position, icon_size.width(), x_axis_position);
		painter.drawLine(y_axis_position, 0, y_axis_position, icon_size.height());

		painter.setPen(Qt::NoPen);

		// start point
		painter.setBrush(Qt::red);
		const QPoint start(y_axis_position, x_axis_position - curve_scale * curve.valueForProgress(0));
		painter.drawRect(start.x() - 1, start.y() - 1, 3, 3);

		// end point
		painter.setBrush(Qt::blue);
		const QPoint end(y_axis_position + curve_scale, x_axis_position - curve_scale * curve.valueForProgress(1));
		painter.drawRect(end.x() - 1, end.y() - 1, 3, 3);

		// curve
		QPainterPath curve_path(start);
		for (float t = 0; t <= 1.0; t += 1.0 / curve_scale)
		{
			QPoint to;
			to.setX(y_axis_position + curve_scale * t);
			to.setY(x_axis_position - curve_scale * curve.valueForProgress(t));
			curve_path.lineTo(to);
		}
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.strokePath(curve_path, QColor(32, 32, 32));
		painter.setRenderHint(QPainter::Antialiasing, false);

		icons.at(i) = QIcon(pixmap);
	}

	return icons;
}

const QIcon& easingCurveIcon(size_t index)
{
	static std::array<QIcon, QEasingCurve::NCurveTypes - 3> easing_curve_icons = drawIcons();
	return easing_curve_icons.at(index);
}

}
