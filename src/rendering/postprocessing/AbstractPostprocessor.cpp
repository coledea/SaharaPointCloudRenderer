#include "AbstractPostprocessor.h"

#include "PostprocessorSpecifications.h"

namespace sahara::rendering
{

AbstractPostprocessor::AbstractPostprocessor(rendering::OpenGLContext* opengl_context) noexcept
	: m_opengl_context(opengl_context)
{
}

std::vector<AbstractParameter*>& AbstractPostprocessor::parameters() noexcept
{
	return m_parameters;
}

const std::set<geometry::AttributeSpecification> AbstractPostprocessor::requiredAttributes() const
{
	return PostprocessorSpecifications::Specifications.at(type()).necessaryAttributes();
}

}
