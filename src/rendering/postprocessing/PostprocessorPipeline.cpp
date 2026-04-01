#include "PostprocessorPipeline.h"

namespace sahara::rendering
{
PostprocessorPipeline::PostprocessorPipeline()
	: m_shader_specifications_changed(false)
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
	recomputeShaderSpecifications();
}

void PostprocessorPipeline::removePostprocessor(uint index)
{
	assert(index < m_postprocessors.size());
	m_postprocessors.erase(m_postprocessors.begin() + index);
	recomputeShaderSpecifications();
}

size_t PostprocessorPipeline::numberOfPostprocessors() const noexcept
{
	return m_postprocessors.size();
}

const PostprocessorShaderSpecifications& PostprocessorPipeline::shaderSpecifications() const noexcept
{
	return m_combined_shader_specifications;
}

std::vector<AbstractParameter*>& PostprocessorPipeline::postprocessorParameters(uint index) const
{
	assert(index < m_postprocessors.size());
	return m_postprocessors[index]->parameters();
}

bool PostprocessorPipeline::shaderSpecificationsChanged()
{
	return std::exchange(m_shader_specifications_changed, false);
}

void PostprocessorPipeline::recomputeShaderSpecifications()
{
	std::set<geometry::AttributeSpecification> required_vertex_attributes;
	for (const auto& postprocessor : m_postprocessors)
	{
		required_vertex_attributes.insert(postprocessor->shaderSpecifications().required_vertex_attributes.begin(),
										  postprocessor->shaderSpecifications().required_vertex_attributes.end());
	}

	// only update the shader specifications if they have really changed, as this might trigger shader recompilations
	if (!geometry::attributeSpecificationsEqual(m_combined_shader_specifications.required_vertex_attributes, required_vertex_attributes))
	{
		m_combined_shader_specifications.required_vertex_attributes.insert(required_vertex_attributes.begin(), required_vertex_attributes.end());
		m_shader_specifications_changed = true;
	}
}

}