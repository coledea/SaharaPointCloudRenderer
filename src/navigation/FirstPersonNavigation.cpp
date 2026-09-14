#include "FirstPersonNavigation.h"

namespace sahara::navigation
{

FirstPersonNavigation::FirstPersonNavigation(Camera* camera)
	: AbstractNavigation(camera)
{
}

void FirstPersonNavigation::onMouseMoveLeftButton(QPointF move, float move_speed, [[maybe_unused]] float look_speed)
{
	const float move_x = -move.x() * 0.0001f * move_speed;
	const float move_y = move.y() * 0.0001f * move_speed;
	m_camera->translate(QVector3D(move_x, move_y, 0.0f));
}

void FirstPersonNavigation::onMouseMoveRightButton(QPointF move, [[maybe_unused]] float move_speed, float look_speed)
{
	const float pan = -move.x() / 100.0f * look_speed;
	const float tilt = -move.y() / 100.0f * look_speed;

	QQuaternion rotation = QQuaternion::fromAxisAndAngle(m_camera->right(), tilt);
	rotation *= QQuaternion::fromAxisAndAngle(m_camera->up(), pan);
	m_camera->rotateAroundEye(rotation);
}

void FirstPersonNavigation::onWheelMove(float delta, float move_speed, [[maybe_unused]] float look_speed)
{
	m_camera->translate(QVector3D(0.0f, 0.0f, delta * 0.01f * move_speed));
}

}