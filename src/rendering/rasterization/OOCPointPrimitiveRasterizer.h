#pragma once

#include "PointPrimitiveRasterizer.h"

namespace sahara::rendering
{

class OOCPointPrimitiveRasterizer : public PointPrimitiveRasterizer
{
	Q_OBJECT

public:
	OOCPointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	~OOCPointPrimitiveRasterizer();

	void run(Framebuffer* framebuffer, int read_fbo_index) override;

	RasterizerType type() const noexcept override;
	static std::set<geometry::AttributeSpecification> necessaryAttributes() noexcept;

private:
	ShaderPaths getShaderPaths() const noexcept override;
	void setVertexAttribute(const geometry::AttributeSpecification& attribute) override;
};

}