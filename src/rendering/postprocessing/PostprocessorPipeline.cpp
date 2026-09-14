#include "PostprocessorPipeline.h"

#include "PostprocessorSpecifications.h"

namespace sahara::rendering
{
PostprocessorPipeline::PostprocessorPipeline()
{
}

void PostprocessorPipeline::run(Framebuffer* framebuffer, int read_fbo_index)
{
	for (const auto& postprocessor : m_postprocessors)
	{
		postprocessor->run(framebuffer, read_fbo_index);
		read_fbo_index = 1 - read_fbo_index; // ping-pong rendering
	}
}

void PostprocessorPipeline::reloadShaders()
{
	for (const auto& postprocessor : m_postprocessors)
	{
		postprocessor->reloadShader();
	}
}

void PostprocessorPipeline::appendPostprocessor(std::unique_ptr<AbstractPostprocessor> postprocessor)
{
	m_postprocessors.push_back(std::move(postprocessor));
	recomputeRequirements();
}

void PostprocessorPipeline::removePostprocessor(uint index)
{
	assert(index < m_postprocessors.size());
	m_postprocessors.erase(m_postprocessors.begin() + index);
	recomputeRequirements();
	emit postprocessorRemoved(index);
}

void PostprocessorPipeline::removeIncompatiblePostprocessors(const std::set<geometry::AttributeSpecification>& available_attributes)
{
	for (auto it = m_postprocessors.begin(); it != m_postprocessors.end(); ++it)
	{
		auto required_attributes = it->get()->requiredAttributes();
		if (std::any_of(required_attributes.begin(), required_attributes.end(), [available_attributes](const auto& attribute) {
				return !available_attributes.contains(attribute);
			}))
		{
			auto index = std::distance(m_postprocessors.begin(), it);
			removePostprocessor(index);
			continue;
		}
	}
	recomputeRequirements();
}

size_t PostprocessorPipeline::numberOfPostprocessors() const noexcept
{
	return m_postprocessors.size();
}

const std::set<geometry::AttributeSpecification>& PostprocessorPipeline::requiredAttributes() const noexcept
{
	return m_required_attributes;
}

const std::set<FramebufferAttachmentTypes>& PostprocessorPipeline::requiredFramebufferAttachments() const noexcept
{
	return m_required_framebuffer_attachments;
}

std::vector<AbstractParameter*>& PostprocessorPipeline::postprocessorParameters(uint index) const
{
	assert(index < m_postprocessors.size());
	return m_postprocessors[index]->parameters();
}

void PostprocessorPipeline::recomputeRequirements()
{
	std::set<geometry::AttributeSpecification> combined_required_attributes;
	std::set<FramebufferAttachmentTypes> combined_required_framebuffer_attachments;
	for (const auto& postprocessor : m_postprocessors)
	{
		const auto required_attributes = postprocessor->requiredAttributes();
		combined_required_attributes.insert(required_attributes.begin(), required_attributes.end());

		const auto& required_attachments = PostprocessorSpecifications::Specifications.at(postprocessor->type()).requiredFramebufferAttachments();
		combined_required_framebuffer_attachments.insert(required_attachments.begin(), required_attachments.end());
	}

	const bool attributes_changed = !geometry::attributeSpecificationsEqual(m_required_attributes, combined_required_attributes);
	const bool attachments_changed = m_required_framebuffer_attachments != combined_required_framebuffer_attachments;
	if (attributes_changed || attachments_changed)
	{
		m_required_attributes = std::move(combined_required_attributes);
		m_required_framebuffer_attachments = std::move(combined_required_framebuffer_attachments);
		emit inputsUpdated();
	}
}

}