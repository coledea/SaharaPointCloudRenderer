#include "FirstPersonNavigation.h"

#include "ui/RenderWindow.h"

namespace sahara::navigation
{

FirstPersonNavigation::FirstPersonNavigation(Camera* camera)
	: AbstractNavigation(camera)
{
}

void FirstPersonNavigation::update(float move_speed, float look_speed)
{
	auto events = ui::RenderWindow::s_event_cache;

	if (events.didMouseMoveOccur() && events.isMouseButtonPressed(Qt::RightButton))
	{
		onMouseMoveRightButton(events.getMouseMovement(), look_speed);
	}

	if (events.isKeyPressed(Qt::Key_W))
	{
		m_camera->translate(QVector3D(0.0f, 0.0f, 0.005f * move_speed));
	}
	if (events.isKeyPressed(Qt::Key_S))
	{
		m_camera->translate(QVector3D(0.0f, 0.0f, -0.005f * move_speed));
	}
	if (events.isKeyPressed(Qt::Key_A))
	{
		m_camera->translate(QVector3D(-0.005f * move_speed, 0.0f, 0.0f));
	}
	if (events.isKeyPressed(Qt::Key_D))
	{
		m_camera->translate(QVector3D(0.005f * move_speed, 0.0f, 0.0f));
	}
}

void FirstPersonNavigation::onMouseMoveRightButton(QPointF move, float look_speed)
{
	const float pan = -move.x() / 100.0f * look_speed;
	const float tilt = -move.y() / 100.0f * look_speed;

	QQuaternion rotation = QQuaternion::fromAxisAndAngle(m_camera->right(), tilt);
	rotation *= QQuaternion::fromAxisAndAngle(m_camera->up(), pan);
	m_camera->rotateAroundEye(rotation);
}
}