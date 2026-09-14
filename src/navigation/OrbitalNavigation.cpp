#include "OrbitalNavigation.h"

#include "ui/RenderWindow.h"

namespace sahara::navigation
{

OrbitalNavigation::OrbitalNavigation(Camera* camera)
	: AbstractNavigation(camera)
{
}

void OrbitalNavigation::update(float move_speed, float look_speed)
{
	auto events = ui::RenderWindow::s_event_cache;

	if (events.didMouseMoveOccur())
	{
		if (events.isMouseButtonPressed(Qt::LeftButton))
		{
			onMouseMoveLeftButton(events.getMouseMovement(), move_speed);
		}
		if (events.isMouseButtonPressed(Qt::RightButton))
		{
			onMouseMoveRightButton(events.getMouseMovement(), look_speed);
		}
	}

	if (events.didWheelOccur())
	{
		float delta = events.getWheelPixelDelta() ? events.getWheelPixelDelta()->y() : static_cast<float>(events.getWheelAngleDelta().y() / 120);
		onWheelMove(delta, move_speed);
	}
}

void OrbitalNavigation::onMouseMoveLeftButton(QPointF move, float move_speed)
{
	const float move_x = -move.x() * 0.0001f * move_speed;
	const float move_y = move.y() * 0.0001f * move_speed;
	m_camera->translate(QVector3D(move_x, move_y, 0.0));
}

void OrbitalNavigation::onMouseMoveRightButton(QPointF move, float look_speed)
{
	const float pan = move.x() / 100.0f * look_speed;
	const float tilt = move.y() / 100.0f * look_speed;

	QQuaternion rotation = QQuaternion::fromAxisAndAngle(m_camera->right(), tilt);
	rotation *= QQuaternion::fromAxisAndAngle(m_camera->up(), pan);
	m_camera->rotateAroundCenter(rotation);
}

void OrbitalNavigation::onWheelMove(float delta, float move_speed)
{
	m_camera->zoom(delta * 0.01f * move_speed);
}

}