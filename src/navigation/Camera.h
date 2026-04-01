#pragma once

#include "geometry/BoundingBox.h"
#include "utils/CachedValue.h"

#include <QMatrix4x4>
#include <QObject>
#include <QQuaternion>

namespace sahara::navigation
{

struct CameraSpecification
{
	CameraSpecification(const QVector3D& eye, const QVector3D& center, const QVector3D& up, float near_plane, float far_plane, float fov) noexcept
		: eye(eye)
		, center(center)
		, up(up)
		, near_plane(near_plane)
		, far_plane(far_plane)
		, fov(fov)
	{
	}

	CameraSpecification() noexcept
		: CameraSpecification(QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), 0.2f, 100.0f, 45.0f)
	{
	}

	QVector3D eye;
	QVector3D center;
	QVector3D up;
	float near_plane;
	float far_plane;
	float fov;
};

class Camera : public QObject
{
	Q_OBJECT

public:
	Camera() noexcept;
	Camera(uint viewport_width, uint viewport_height, const CameraSpecification& spec);

	// Setter
	void setViewport(uint width, uint height);
	void setCameraSpecification(const CameraSpecification& spec);
	void setOptimalViewForBounds(const geometry::BoundingBox& bbox); // sets an isometric view for the given bbox.

	// Getter
	uint viewportWidth() const noexcept;
	uint viewportHeight() const noexcept;
	float viewportDiagonalLength() const noexcept;
	float focalLength() const noexcept;
	const CameraSpecification& cameraSpecifications() const noexcept;

	const QVector3D& up() const noexcept;
	const QVector3D right() const noexcept;
	const QVector3D forward() const noexcept;

	const QMatrix4x4& viewMatrix() const;
	const QMatrix4x4& projectionMatrix() const;
	const QMatrix4x4& viewProjectionMatrix() const;

	const QVector3D extrapolateEyeFromMovement(float t) const noexcept;
	const QVector3D extrapolateForwardFromMovement(float t) const noexcept;

	// Camera transformation
	void translate(const QVector3D& translation);
	void translateWorld(const QVector3D& translation);

	void zoom(float value);

	void rotateAroundCenter(float angle, const QVector3D& axis);
	void rotateAroundCenter(const QQuaternion& rotation);
	void rotateAroundEye(float angle, const QVector3D& axis);
	void rotateAroundEye(const QQuaternion& rotation);

signals:
	// for informing the camera settings panel when the camera settings changed due to camera path animations
	void fovChanged(float fov);
	void farPlaneChanged(float far_plane);
	void nearPlaneChanged(float near_plane);

public slots:
	void setNearPlane(float near_plane);
	void setFarPlane(float far_plane);
	void setFov(float fov);

private:
	void computeViewMatrix(QMatrix4x4& result) const;
	void computeProjectionMatrix(QMatrix4x4& result) const;

	uint m_viewport_width;
	uint m_viewport_height;
	float m_viewport_diagonal_length;
	float m_aspect_ratio;
	float m_focal_length;

	CameraSpecification m_camera_specification;

	QVector3D m_eye_move_direction;
	QVector3D m_center_move_direction;
	const float m_movement_smoothing_factor = 0.5f;

	utils::CachedValue<QMatrix4x4> m_view_matrix;
	utils::CachedValue<QMatrix4x4> m_projection_matrix;
	utils::CachedValue<QMatrix4x4> m_view_projection_matrix;
};

}
