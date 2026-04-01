#pragma once

#include <QVector3D>

namespace sahara::geometry
{

class BoundingBox
{
public:
	BoundingBox() noexcept;
	BoundingBox(const QVector3D& min, const QVector3D& max);

	const QVector3D& minimum() const noexcept;
	const QVector3D& maximum() const noexcept;
	QVector3D center() const noexcept;
	QVector3D extent() const noexcept;

	bool contains(const QVector3D& point) const noexcept;
	bool intersects(const BoundingBox& bbox) const noexcept;

	void extendToIncludePoint(const QVector3D& point);
	void setBounds(const QVector3D& min, const QVector3D& max);

	float projectedSize(const QMatrix4x4& view_matrix, const QMatrix4x4& projection_matrix, int width, int height) const; // NDC size

private:
	QVector3D m_min;
	QVector3D m_max;
};

}