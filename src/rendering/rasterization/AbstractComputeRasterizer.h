#pragma once

#include "AbstractRasterizer.h"
#include "rendering/ShaderProgramFactory.h"
#include "rendering/parameters/RangeParameter.h"
#include "utils/FullscreenGeometry.h"

#include <QOpenGLShaderProgram>

namespace sahara::rendering
{

class AbstractComputeRasterizer : public AbstractRasterizer
{
	Q_OBJECT

public:
	AbstractComputeRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~AbstractComputeRasterizer();

	virtual void recompileShaders(const std::set<geometry::AttributeSpecification>& required_outputs) override;
	virtual void run(Framebuffer* framebuffer, int read_fbo_index) override;

	static std::set<geometry::AttributeSpecification> necessaryAttributes() noexcept;

protected slots:
	void onPointSizeChanged();

protected:
	std::unique_ptr<utils::FullscreenGeometry> m_fullscreen_geometry;
	std::unique_ptr<QOpenGLShaderProgram> m_clear_program;
	std::unique_ptr<QOpenGLShaderProgram> m_resolve_program;
	std::unique_ptr<RangeParameter<int>> m_point_size_parameter;
	std::set<geometry::AttributeSpecification> m_attributes;

	virtual void createShaderPrograms(ShaderProgramFactory& factory, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification>& required_outputs) = 0;
	virtual void bindGPUBuffers();
	virtual void updateIntermediateBuffers(int width, int height) = 0;
	inline virtual int getFirstAttributeSSBOBindingLocation() const noexcept = 0;
	virtual void bindGPUBuffer(const geometry::AttributeSpecification& attribute); // requires current context and shader program being bound

	virtual void runPrograms(Framebuffer* framebuffer, int read_fbo_index) = 0;

	void runClearProgram(Framebuffer* framebuffer, int read_fbo_index);
	void runResolveProgram(Framebuffer* framebuffer, int read_fbo_index);
};

}