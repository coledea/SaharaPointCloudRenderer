#pragma once

#include "geometry/AttributeMetadata.h"
#include "rendering/Framebuffer.h"
#include "rendering/RendererModuleTypes.h"
#include "rendering/parameters/AbstractParameter.h"

#include <set>

namespace sahara::rendering
{

class AbstractPostprocessor
{
public:
	AbstractPostprocessor(OpenGLContext* opengl_context) noexcept;
	virtual ~AbstractPostprocessor() = default;

	virtual void reloadShader() = 0;
	virtual void run(Framebuffer* framebuffer, int read_fbo_index) = 0;
	virtual PostprocessorType type() const noexcept = 0;

	std::vector<AbstractParameter*>& parameters() noexcept;

	virtual const std::set<geometry::AttributeSpecification> requiredAttributes() const;

protected:
	OpenGLContext* m_opengl_context;
	std::vector<AbstractParameter*> m_parameters;
};

}