#pragma once

#include "AbstractRasterizer.h"
#include "rendering/parameters/RangeParameter.h"

#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class PointPrimitiveRasterizer : public QObject, public AbstractRasterizer
{
	Q_OBJECT

public:
	PointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~PointPrimitiveRasterizer();

	virtual void reloadShaderSpecificationsFromDisk() override;
	virtual void setCompiledShaderProgram(QOpenGLShaderProgram* shader_program, const std::set<geometry::AttributeSpecification>& input_attributes) override;
	virtual void run() override;

	virtual RasterizerType type() const noexcept override;

protected slots:
	void onPointSizeChanged();

protected:
	QOpenGLVertexArrayObject m_vao; // Vertex Array Object (VAO).
	std::unique_ptr<RangeParameter<int>> m_point_size_parameter;

	virtual void initializeVAO();
	virtual void setVertexAttribute(const geometry::AttributeSpecification& attribute); // requires current context and shader program being bound
};

}