#include "CameraAnimationStep.h"

#include "utils/QVector3DUtilities.h"

namespace sahara::navigation
{

CameraAnimationStep::CameraAnimationStep(Camera* camera, const CameraAnimationKeyframe& source, const CameraAnimationKeyframe& target)
	: QAbstractAnimation(nullptr)
	, m_camera(camera)
	, m_source(source)
	, m_target(target)
	, m_source_orientation(QQuaternion::fromDirection(m_source.camera.center - m_source.camera.eye, m_source.camera.up))
	, m_target_orientation(QQuaternion::fromDirection(m_target.camera.center - m_target.camera.eye, m_target.camera.up))
{
}

int CameraAnimationStep::duration() const noexcept
{
	return m_source.duration;
}

void CameraAnimationStep::updateCurrentTime(int currentTime)
{
	const float progress = m_source.easing.valueForProgress(static_cast<float>(currentTime) / static_cast<float>(m_source.duration));

	auto new_orientation = QQuaternion::slerp(m_source_orientation, m_target_orientation, progress);

	CameraSpecification new_specifications = m_camera->cameraSpecifications();
	new_specifications.eye = lerp(m_source.camera.eye, m_target.camera.eye, progress);
	new_specifications.center = new_specifications.eye + new_orientation.rotatedVector(QVector3D(0, 0, 1));
	new_specifications.up = new_orientation.rotatedVector(QVector3D(0, 1, 0));
	new_specifications.fov = std::lerp(m_source.camera.fov, m_target.camera.fov, progress);
	m_camera->setCameraSpecification(new_specifications);
}

}