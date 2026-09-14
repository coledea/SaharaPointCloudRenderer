#pragma once

#include "AbstractComputeRasterizer.h"
#include "rendering/parameters/RangeParameter.h"
#include "utils/FullscreenGeometry.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class PointComputeRasterizer : public AbstractComputeRasterizer
{
public:
	PointComputeRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~PointComputeRasterizer();

	virtual RasterizerType type() const noexcept override;

protected:
	int m_intermediate_buffer_size;
	QOpenGLBuffer m_intermediate_buffer;
	std::unique_ptr<QOpenGLShaderProgram> m_render_program;

	virtual void createShaderPrograms(ShaderProgramFactory& factory, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification>& required_outputs) override;
	virtual void bindGPUBuffers() override;
	virtual void updateIntermediateBuffers(int width, int height) override;
	inline virtual int getFirstAttributeSSBOBindingLocation() const noexcept override;
	virtual void runPrograms(Framebuffer* framebuffer, int read_fbo_index) override;

private:
	void runRenderProgram(Framebuffer* framebuffer, int read_fbo_index);
};

}