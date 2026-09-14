#pragma once

#include "AbstractPostprocessor.h"

namespace sahara::rendering
{

// manages multiple postprocessors that are executed in sequence
class PostprocessorPipeline : public QObject
{
	Q_OBJECT

public:
	PostprocessorPipeline();

	size_t numberOfPostprocessors() const noexcept;
	const std::set<geometry::AttributeSpecification>& requiredAttributes() const noexcept;
	const std::set<FramebufferAttachmentTypes>& requiredFramebufferAttachments() const noexcept;
	std::vector<AbstractParameter*>& postprocessorParameters(uint index) const;

	void appendPostprocessor(std::unique_ptr<AbstractPostprocessor> postprocessor);
	void removePostprocessor(uint index);
	void removeIncompatiblePostprocessors(const std::set<geometry::AttributeSpecification>& available_attributes);

	void run(Framebuffer* framebuffer, int read_fbo_index);
	void reloadShaders(); // loads all sources again from disk and compiles all shaders

signals:
	void inputsUpdated();
	void postprocessorRemoved(int index);

protected:
	std::vector<std::unique_ptr<AbstractPostprocessor>> m_postprocessors;
	std::set<geometry::AttributeSpecification> m_required_attributes;
	std::set<FramebufferAttachmentTypes> m_required_framebuffer_attachments;

	void recomputeRequirements();
};

}