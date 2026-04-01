#include "BoundingBox.h"

#include "utils/QVector3DUtilities.h"

#include <limits>

namespace sahara::geometry
{

BoundingBox::BoundingBox() noexcept
	: BoundingBox(QVector3D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), QVector3D(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()))
{
}

BoundingBox::BoundingBox(const QVector3D& min, const QVector3D& max)
	: m_min(min)
	, m_max(max)
{
}

const QVector3D& BoundingBox::minimum() const noexcept
{
	return m_min;
}

const QVector3D& BoundingBox::maximum() const noexcept
{
	return m_max;
}

QVector3D BoundingBox::center() const noexcept
{
	return m_min + extent() / 2.0f;
}

QVector3D BoundingBox::extent() const noexcept
{
	return m_max - m_min;
}

bool BoundingBox::contains(const QVector3D& point) const noexcept
{
	return point.x() >= m_min.x() && point.x() <= m_max.x() && point.y() >= m_min.y() && point.y() <= m_max.y() && point.z() >= m_min.z() && point.z() <= m_max.z();
}

bool BoundingBox::intersects(const BoundingBox& bbox) const noexcept
{
	return m_min.x() <= bbox.m_max.x() && m_max.x() >= bbox.m_min.x() && m_min.y() <= bbox.m_max.y() && m_max.y() >= bbox.m_min.y() && m_min.z() <= bbox.m_max.z() && m_max.z() >= bbox.m_min.z();
}

void BoundingBox::extendToIncludePoint(const QVector3D& point)
{
	m_min = min(m_min, point);
	m_max = max(m_max, point);
}

void BoundingBox::setBounds(const QVector3D& min, const QVector3D& max)
{
	m_min = min;
	m_max = max;
}

// the projected size, assuming the camera is not inside the bounding box
float BoundingBox::projectedSize(const QMatrix4x4& view_matrix, const QMatrix4x4& projection_matrix, int width, int height) const
{
	QVector2D minimum(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	QVector2D maximum(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	for (int i = 0; i < 8; i++)
	{
		QVector3D mask((i & 0b100) >> 2, (i & 0b010) >> 1, (i & 0b001));
		QVector3D corner(mask * m_min + (QVector3D(1.0, 1.0, 1.0) - mask) * m_max);

		auto projected = corner.project(view_matrix, projection_matrix, QRect(0, 0, width, height)) / QVector3D(width, height, 1.0);
		// projected = max(min(projected, QVector3D(1.0, 1.0, 1.0)), QVector3D(0.0, 0.0, 0.0));
		if (projected.z() <= 0.0)
		{
			continue; // exclude points behind the camera
		}
		minimum = min(minimum, projected.toVector2D());
		maximum = max(maximum, projected.toVector2D());
	}

	minimum = max(min(minimum, QVector2D(1.0, 1.0)), QVector2D(0.0, 0.0));
	maximum = max(min(maximum, QVector2D(1.0, 1.0)), QVector2D(0.0, 0.0));
	return (maximum.x() - minimum.x()) * (maximum.y() - minimum.y()); // size of the AABB of the projected points on screen
}

}
