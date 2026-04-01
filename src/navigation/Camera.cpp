#include "Camera.h"

namespace sahara::navigation
{

Camera::Camera() noexcept
	: Camera(1920, 1080, CameraSpecification())
{
}

Camera::Camera(uint viewport_width, uint viewport_height, const CameraSpecification& spec)
	: m_viewport_width(viewport_width)
	, m_viewport_height(viewport_height)
	, m_viewport_diagonal_length(std::hypot(static_cast<float>(viewport_width), static_cast<float>(viewport_height)))
	, m_focal_length(0.5f * viewport_width / std::tan(qDegreesToRadians(spec.fov) * 0.5f))
	, m_aspect_ratio(static_cast<float>(viewport_width) / static_cast<float>(viewport_height))
	, m_camera_specification(spec)
	, m_eye_move_direction(QVector3D(0.0f, 0.0f, 0.0f))
	, m_center_move_direction(QVector3D(0.0f, 0.0f, 0.0f))
	, m_view_matrix([this](QMatrix4x4& result) { computeViewMatrix(result); })
	, m_projection_matrix([this](QMatrix4x4& result) { computeProjectionMatrix(result); })
	, m_view_projection_matrix([this](QMatrix4x4& result) { result = projectionMatrix() * viewMatrix(); })
{
}

void Camera::setViewport(uint viewport_width, uint viewport_height)
{
	m_viewport_width = viewport_width;
	m_viewport_height = viewport_height;
	m_viewport_diagonal_length = std::hypot(static_cast<float>(viewport_width), static_cast<float>(viewport_height));
	m_aspect_ratio = static_cast<float>(viewport_width) / static_cast<float>(viewport_height);
	m_projection_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

void Camera::setNearPlane(float near_plane)
{
	if (near_plane == m_camera_specification.near_plane)
	{
		return;
	}

	m_camera_specification.near_plane = near_plane;
	m_projection_matrix.invalidate();
	m_view_projection_matrix.invalidate();
	emit nearPlaneChanged(near_plane);
}

void Camera::setFarPlane(float far_plane)
{
	if (far_plane == m_camera_specification.far_plane)
	{
		return;
	}

	m_camera_specification.far_plane = far_plane;
	m_projection_matrix.invalidate();
	m_view_projection_matrix.invalidate();
	emit farPlaneChanged(far_plane);
}

void Camera::setFov(float fov)
{
	if (fov == m_camera_specification.fov)
	{
		return;
	}

	m_camera_specification.fov = fov;
	m_focal_length = 0.5f * m_viewport_width / std::tan(qDegreesToRadians(fov) * 0.5f);
	m_projection_matrix.invalidate();
	m_view_projection_matrix.invalidate();
	emit fovChanged(fov);
}

void Camera::setCameraSpecification(const CameraSpecification& spec)
{
	if (spec.fov != m_camera_specification.fov)
	{
		emit fovChanged(spec.fov);
	}

	if (spec.near_plane != m_camera_specification.near_plane)
	{
		emit nearPlaneChanged(spec.near_plane);
	}

	if (spec.far_plane != m_camera_specification.far_plane)
	{
		emit farPlaneChanged(spec.far_plane);
	}

	m_camera_specification = spec;
	m_eye_move_direction = QVector3D(0.0f, 0.0f, 0.0f);
	m_center_move_direction = QVector3D(0.0f, 0.0f, 0.0f);
	m_focal_length = 0.5f * m_viewport_width / std::tan(qDegreesToRadians(spec.fov) * 0.5f);
	m_view_matrix.invalidate();
	m_projection_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

void Camera::setOptimalViewForBounds(const geometry::BoundingBox& bbox)
{
	const QVector3D diagonal_normalized = QVector3D(1.0, 1.0, 1.0).normalized();
	const QVector3D center = bbox.center();
	const QVector3D extent = bbox.extent();

	m_camera_specification.eye = center + diagonal_normalized * extent.length() * 0.75f;
	m_camera_specification.center = center + QVector3D(0.0f, 0.0f, -extent.z() * 0.2f);

	QVector3D temp_up = QVector3D(0.0f, 0.0f, 1.0f);
	if (std::abs(QVector3D::dotProduct(temp_up, diagonal_normalized)) > 0.95f)
	{
		temp_up = QVector3D(0.0f, 1.0f, 0.0f);
	}
	const QVector3D right = QVector3D::crossProduct(temp_up, diagonal_normalized).normalized();
	m_camera_specification.up = QVector3D::crossProduct(diagonal_normalized, right).normalized();

	const float new_far_plane = (m_camera_specification.eye - bbox.minimum()).length() * 2.0f;
	setFarPlane(new_far_plane);

	m_eye_move_direction = QVector3D(0.0f, 0.0f, 0.0f);
	m_center_move_direction = QVector3D(0.0f, 0.0f, 0.0f);
	m_projection_matrix.invalidate();
	m_view_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

uint Camera::viewportWidth() const noexcept
{
	return m_viewport_width;
}

uint Camera::viewportHeight() const noexcept
{
	return m_viewport_height;
}

float Camera::viewportDiagonalLength() const noexcept
{
	return m_viewport_diagonal_length;
}

float Camera::focalLength() const noexcept
{
	return m_focal_length;
}

const CameraSpecification& Camera::cameraSpecifications() const noexcept
{
	return m_camera_specification;
}

const QVector3D& Camera::up() const noexcept
{
	return m_camera_specification.up;
}

const QVector3D Camera::right() const noexcept
{
	return QVector3D::crossProduct(forward(), m_camera_specification.up).normalized();
}

const QVector3D Camera::forward() const noexcept
{
	return (m_camera_specification.center - m_camera_specification.eye).normalized();
}

const QMatrix4x4& Camera::viewMatrix() const
{
	return m_view_matrix.value();
}

const QMatrix4x4& Camera::projectionMatrix() const
{
	return m_projection_matrix.value();
}

const QMatrix4x4& Camera::viewProjectionMatrix() const
{
	return m_view_projection_matrix.value();
}

const QVector3D Camera::extrapolateEyeFromMovement(float t) const noexcept
{
	return m_camera_specification.eye + t * m_eye_move_direction;
}

const QVector3D Camera::extrapolateForwardFromMovement(float t) const noexcept
{
	return ((m_camera_specification.center + t * m_center_move_direction) - m_camera_specification.eye).normalized();
}

void Camera::translate(const QVector3D& translation)
{
	QVector3D translation_world;
	if (!qFuzzyIsNull(translation.x()))
	{
		translation_world += translation.x() * right();
	}

	if (!qFuzzyIsNull(translation.y()))
	{
		translation_world += translation.y() * up();
	}

	if (!qFuzzyIsNull(translation.z()))
	{
		translation_world += translation.z() * forward();
	}

	translateWorld(translation_world);
}

void Camera::translateWorld(const QVector3D& translation)
{
	m_camera_specification.eye += translation;
	m_camera_specification.center += translation;
	m_eye_move_direction = m_eye_move_direction * m_movement_smoothing_factor + translation * (1.0f - m_movement_smoothing_factor);
	m_center_move_direction = m_center_move_direction * m_movement_smoothing_factor + translation * (1.0f - m_movement_smoothing_factor);
	m_view_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

void Camera::zoom(float value)
{
	float forward_length = (m_camera_specification.center - m_camera_specification.eye).length();
	if (forward_length > value)
	{
		auto translation = value * forward();
		m_camera_specification.eye += translation;
		m_eye_move_direction = m_eye_move_direction * m_movement_smoothing_factor + translation * (1.0f - m_movement_smoothing_factor);
	}
	m_view_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

void Camera::rotateAroundCenter(float angle, const QVector3D& axis)
{
	if (qFuzzyIsNull(angle))
	{
		return;
	}

	const QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, angle);
	rotateAroundCenter(rotation);
}

void Camera::rotateAroundCenter(const QQuaternion& rotation)
{
	m_camera_specification.up = (rotation * m_camera_specification.up).normalized();
	auto new_eye = (rotation * (m_camera_specification.eye - m_camera_specification.center)) + m_camera_specification.center;
	m_eye_move_direction = m_eye_move_direction * m_movement_smoothing_factor + (new_eye - m_camera_specification.eye) * (1.0f - m_movement_smoothing_factor);
	m_camera_specification.eye = new_eye;
	m_view_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

void Camera::rotateAroundEye(float angle, const QVector3D& axis)
{
	if (qFuzzyIsNull(angle))
	{
		return;
	}

	const QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, -angle);
	rotateAroundEye(rotation);
}

void Camera::rotateAroundEye(const QQuaternion& rotation)
{
	m_camera_specification.up = (rotation * m_camera_specification.up).normalized();
	auto new_center = (rotation * (m_camera_specification.center - m_camera_specification.eye)) + m_camera_specification.eye;
	m_center_move_direction = m_center_move_direction * m_movement_smoothing_factor + (new_center - m_camera_specification.center) * (1.0f - m_movement_smoothing_factor);
	m_camera_specification.center = new_center;
	m_view_matrix.invalidate();
	m_view_projection_matrix.invalidate();
}

void Camera::computeViewMatrix(QMatrix4x4& result) const
{
	result.setToIdentity();
	result.lookAt(m_camera_specification.eye, m_camera_specification.center, m_camera_specification.up);
}

void Camera::computeProjectionMatrix(QMatrix4x4& result) const
{
	result.setToIdentity();
	result.perspective(m_camera_specification.fov, m_aspect_ratio, m_camera_specification.near_plane, m_camera_specification.far_plane);
}

}