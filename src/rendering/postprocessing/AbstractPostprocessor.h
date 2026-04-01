#pragma once

#include "geometry/AttributeMetadata.h"
#include "rendering/Framebuffer.h"
#include "rendering/RendererModuleTypes.h"
#include "rendering/parameters/AbstractParameter.h"

#include <set>

namespace sahara::rendering
{

struct PostprocessorShaderSpecifications
{
	std::set<geometry::AttributeSpecification> required_vertex_attributes;
	std::set<FramebufferAttachmentTypes> required_framebuffer_attachments;
};

class AbstractPostprocessor
{
public:
	AbstractPostprocessor(OpenGLContext* opengl_context) noexcept;
	virtual ~AbstractPostprocessor() = default;

	virtual void reloadShader() = 0;
	virtual void run(Framebuffer* framebuffer, int read_fbo_index) = 0;
	virtual PostprocessorType type() const noexcept = 0;

	const PostprocessorShaderSpecifications& shaderSpecifications() const noexcept;
	std::vector<AbstractParameter*>& parameters() noexcept;

protected:
	OpenGLContext* m_opengl_context;
	std::vector<AbstractParameter*> m_parameters;
	PostprocessorShaderSpecifications m_shader_specifications;
};

}