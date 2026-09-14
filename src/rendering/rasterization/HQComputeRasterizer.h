#pragma once

#include "AbstractComputeRasterizer.h"
#include "rendering/parameters/RangeParameter.h"
#include "utils/FullscreenGeometry.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class HQComputeRasterizer : public AbstractComputeRasterizer
{
public:
	HQComputeRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~HQComputeRasterizer();

	virtual RasterizerType type() const noexcept override;

protected slots:
	void onThresholdChanged();

protected:
	int m_intermediate_buffers_size;
	QOpenGLBuffer m_intermediate_depth_buffer;
	QOpenGLBuffer m_intermediate_color_buffer;
	std::unique_ptr<QOpenGLShaderProgram> m_depth_program;
	std::unique_ptr<QOpenGLShaderProgram> m_color_program;
	std::unique_ptr<RangeParameter<float>> m_threshold_parameter;

	virtual void createShaderPrograms(ShaderProgramFactory& factory, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification>& required_outputs) override;
	virtual void bindGPUBuffers() override;
	virtual void updateIntermediateBuffers(int width, int height) override;
	inline virtual int getFirstAttributeSSBOBindingLocation() const noexcept override;
	virtual void runPrograms(Framebuffer* framebuffer, int read_fbo_index) override;

private:
	void runDepthProgram(Framebuffer* framebuffer, int read_fbo_index);
	void runColorProgram(Framebuffer* framebuffer, int read_fbo_index);
};

}