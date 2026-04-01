#pragma once

#include "AbstractPostprocessor.h"

namespace sahara::rendering
{

// manages multiple postprocessors that are executed in sequence
class PostprocessorPipeline
{
public:
	PostprocessorPipeline();

	size_t numberOfPostprocessors() const noexcept;
	const PostprocessorShaderSpecifications& shaderSpecifications() const noexcept; // returns the combined specifications (i.e., required shader defines and input attributes) of all postprocessing steps
	std::vector<AbstractParameter*>& postprocessorParameters(uint index) const;

	void appendPostprocessor(std::unique_ptr<AbstractPostprocessor> postprocessor);
	void removePostprocessor(uint index);

	void run(Framebuffer* framebuffer, int read_fbo_index);
	void reloadShaders(); // loads all sources again from disk and compiles all shaders

	bool shaderSpecificationsChanged(); // returns whether the specs have changed since the last call to this function

protected:
	std::vector<std::unique_ptr<AbstractPostprocessor>> m_postprocessors;
	PostprocessorShaderSpecifications m_combined_shader_specifications;
	bool m_shader_specifications_changed;

	void recomputeShaderSpecifications();
};

}