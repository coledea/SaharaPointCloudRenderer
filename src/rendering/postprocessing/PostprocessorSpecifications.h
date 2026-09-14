#pragma once

#include "geometry/AttributeSpecification.h"
#include "rendering/Framebuffer.h"
#include "rendering/RendererModuleTypes.h"

#include <map>
#include <memory>
#include <set>

namespace sahara::navigation
{
class Camera;
}

namespace sahara::rendering
{

class AbstractPostprocessor;
class OpenGLContext;

class PostprocessorSpecifications
{
public:
	using FactoryMethod = std::unique_ptr<AbstractPostprocessor> (*)(OpenGLContext* opengl_context, navigation::Camera* camera);

	PostprocessorSpecifications(PostprocessorType type, std::set<geometry::AttributeSpecification> necessary_attributes, std::set<FramebufferAttachmentTypes> required_framebuffer_attachments, FactoryMethod factory_method) noexcept;

	PostprocessorType type() const noexcept;
	const std::set<geometry::AttributeSpecification>& necessaryAttributes() const noexcept;
	const std::set<FramebufferAttachmentTypes>& requiredFramebufferAttachments() const noexcept;
	std::unique_ptr<AbstractPostprocessor> createPostprocessor(OpenGLContext* opengl_context, navigation::Camera* camera) const;

	static const std::map<PostprocessorType, PostprocessorSpecifications> Specifications;

private:
	PostprocessorType m_type;
	std::set<geometry::AttributeSpecification> m_necessary_attributes;
	std::set<FramebufferAttachmentTypes> m_required_framebuffer_attachments;
	FactoryMethod m_factory_method;
};

}
