#pragma once

#include "AbstractRasterizer.h"
#include "rendering/ShaderProgramFactory.h"
#include "rendering/parameters/RangeParameter.h"

#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class PointPrimitiveRasterizer : public AbstractRasterizer
{
	Q_OBJECT

public:
	PointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~PointPrimitiveRasterizer();

	virtual void recompileShaders(const std::set<geometry::AttributeSpecification>& required_outputs) override;
	virtual void run(Framebuffer* framebuffer, int read_fbo_index) override;

	virtual RasterizerType type() const noexcept override;
	static std::set<geometry::AttributeSpecification> necessaryAttributes() noexcept;

protected slots:
	void onPointSizeChanged();

protected:
	QOpenGLVertexArrayObject m_vao; // Vertex Array Object (VAO).
	std::unique_ptr<QOpenGLShaderProgram> m_shader_program;
	std::unique_ptr<RangeParameter<int>> m_point_size_parameter;

	virtual ShaderPaths getShaderPaths() const noexcept;
	virtual void initializeVAO();
	virtual void setVertexAttribute(const geometry::AttributeSpecification& attribute); // requires current context and shader program being bound
};

}