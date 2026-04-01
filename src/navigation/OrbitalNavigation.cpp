#include "OrbitalNavigation.h"

namespace sahara::navigation
{

OrbitalNavigation::OrbitalNavigation(Camera* camera)
	: AbstractNavigation(camera)
{
}

void OrbitalNavigation::onMouseMoveLeftButton(QPoint move, float move_speed, [[maybe_unused]] float look_speed)
{
	const float move_x = -static_cast<float>(move.x()) * 0.0001f * move_speed;
	const float move_y = static_cast<float>(move.y()) * 0.0001f * move_speed;
	m_camera->translate(QVector3D(move_x, move_y, 0.0));
}

void OrbitalNavigation::onMouseMoveRightButton(QPoint move, [[maybe_unused]] float move_speed, float look_speed)
{
	const float pan = static_cast<float>(move.x()) / 100.0f * look_speed;
	const float tilt = static_cast<float>(move.y()) / 100.0f * look_speed;

	QQuaternion rotation = QQuaternion::fromAxisAndAngle(m_camera->right(), tilt);
	rotation *= QQuaternion::fromAxisAndAngle(m_camera->up(), pan);
	m_camera->rotateAroundCenter(rotation);
}

void OrbitalNavigation::onWheelMove(float delta, float move_speed, [[maybe_unused]] float look_speed)
{
	m_camera->zoom(delta * 0.01f * move_speed);
}

}