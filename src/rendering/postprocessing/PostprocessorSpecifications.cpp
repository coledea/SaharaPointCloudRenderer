#include "PostprocessorSpecifications.h"

#include "EyeDomeLightingPostprocessor.h"
#include "HoleFillingPostprocessor.h"

#include <cassert>
#include <utility>

namespace sahara::rendering
{

const std::map<PostprocessorType, PostprocessorSpecifications> PostprocessorSpecifications::Specifications = {
	{ PostprocessorType::EyeDomeLighting,
	  PostprocessorSpecifications(PostprocessorType::EyeDomeLighting, {}, { FramebufferAttachmentTypes::COLOR, FramebufferAttachmentTypes::DEPTH }, [](OpenGLContext* opengl_context, navigation::Camera* camera) -> std::unique_ptr<AbstractPostprocessor> {
		  return std::make_unique<EyeDomeLightingPostprocessor>(opengl_context, camera);
	  }) },
	{ PostprocessorType::HoleFilling, PostprocessorSpecifications(PostprocessorType::HoleFilling, {}, { FramebufferAttachmentTypes::COLOR, FramebufferAttachmentTypes::DEPTH }, [](OpenGLContext* opengl_context, navigation::Camera*) -> std::unique_ptr<AbstractPostprocessor> {
		  return std::make_unique<HoleFillingPostprocessor>(opengl_context);
	  }) }
};

PostprocessorSpecifications::PostprocessorSpecifications(PostprocessorType type, std::set<geometry::AttributeSpecification> necessary_attributes, std::set<FramebufferAttachmentTypes> required_framebuffer_attachments, FactoryMethod factory_method) noexcept
	: m_type(type)
	, m_necessary_attributes(std::move(necessary_attributes))
	, m_required_framebuffer_attachments(std::move(required_framebuffer_attachments))
	, m_factory_method(factory_method)
{
}

PostprocessorType PostprocessorSpecifications::type() const noexcept
{
	return m_type;
}

const std::set<geometry::AttributeSpecification>& PostprocessorSpecifications::necessaryAttributes() const noexcept
{
	return m_necessary_attributes;
}

const std::set<FramebufferAttachmentTypes>& PostprocessorSpecifications::requiredFramebufferAttachments() const noexcept
{
	return m_required_framebuffer_attachments;
}

std::unique_ptr<AbstractPostprocessor> PostprocessorSpecifications::createPostprocessor(OpenGLContext* opengl_context, navigation::Camera* camera) const
{
	assert(m_factory_method != nullptr);
	return m_factory_method(opengl_context, camera);
}

}
