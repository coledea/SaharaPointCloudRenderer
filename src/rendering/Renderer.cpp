#include "Renderer.h"

#include "pointcloudProviders/MultiTemporalPointCloudProvider.h"
#include "rasterization/multitemporal/MultiTemporalPointPrimitiveRasterizer.h"
#include "rendering/colorization/ColorizerSpecifications.h"
#include "rendering/postprocessing/PostprocessorSpecifications.h"
#include "rendering/rasterization/RasterizerSpecifications.h"

namespace sahara::rendering
{

Renderer::Renderer(AbstractPointCloudProvider* pointcloud_provider, OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera) noexcept
	: m_pointcloud_provider(pointcloud_provider)
	, m_opengl_context(opengl_context)
	, m_framebuffer(framebuffer)
	, m_camera(camera)
	, m_rasterizer(nullptr)
{
	switch (pointcloud_provider->type())
	{
		case PointCloudProviderType::OOCMultiTemporalPointCloudProvider:
		case PointCloudProviderType::OOCSinglePointCloudProvider:
			changeRasterizer(RasterizerType::OOCPointPrimitiveRasterizer);
			break;
		case PointCloudProviderType::MultiTemporalPointCloudProvider:
			changeRasterizer(RasterizerType::MultiTemporalPointPrimitiveRasterizer);
			break;
		default:
			changeRasterizer(RasterizerType::PointPrimitiveRasterizer);
			break;
	}

	connect(&m_postprocessor_pipeline, &PostprocessorPipeline::postprocessorRemoved, this, [this](int index) { emit postprocessorRemoved(index); });
	connect(&m_postprocessor_pipeline, &PostprocessorPipeline::inputsUpdated, this, &Renderer::postprocessorInputsUpdated);
}

void Renderer::render()
{
	m_pointcloud_provider->update();
	m_rasterizer->run(m_framebuffer, m_framebuffer->currentFBOIndex());
	m_postprocessor_pipeline.run(m_framebuffer, m_framebuffer->currentFBOIndex());
	// After a point cloud is rendered, the active FBO might need to change, depending on the number of postprocessors in the previous renderer.
	// We assume that each postprocessor reads the textures from one FBO and writes to the other in alternating ways.
	// For an odd number of postprocessors, the active FBO (the one that gets blit to the backbuffer or into which the next point cloud is rendered) has to change.
	if (numberOfPostprocessors() % 2 == 1)
	{
		m_framebuffer->swapCurrentFBO();
	}
}

void Renderer::reloadShader()
{
	const auto& postprocessor_required_attributes = m_postprocessor_pipeline.requiredAttributes();
	m_pointcloud_provider->setRequiredAttributes(m_rasterizer->requiredAttributes(postprocessor_required_attributes)); // this creates the required GPU buffers. We need to call it before informing the rasterizer.

	m_rasterizer->recompileShaders(postprocessor_required_attributes);
	m_postprocessor_pipeline.reloadShaders();
}

bool Renderer::rasterizerFullfillsRequirements(std::set<geometry::AttributeSpecification> required_attributes)
{
	const auto available_attributes = m_rasterizer->availableAttributes();
	return std::all_of(required_attributes.begin(), required_attributes.end(), [available_attributes](const auto& attribute) {
		return available_attributes.contains(attribute);
	});
}

bool Renderer::isModuleTypeSupported(RendererModule module, int module_type)
{

	if (module == RendererModule::Rasterizer)
	{
		const auto supported_rasterizers = m_pointcloud_provider->supportedRasterizers();
		const auto rasterizer_type = static_cast<RasterizerType>(module_type);
		if (!std::any_of(supported_rasterizers.begin(), supported_rasterizers.end(), [rasterizer_type](const auto& attribute) -> bool { return attribute == rasterizer_type; }))
		{
			return false;
		}

		const auto& required_attributes = RasterizerSpecifications::Specifications.at(rasterizer_type).necessaryAttributes();
		return std::all_of(required_attributes.begin(), required_attributes.end(), [this](const auto& attribute) {
			return m_pointcloud_provider->hasAttribute(attribute.semantic);
		});
	}
	else if (module == RendererModule::Colorizer)
	{
		const auto colorizer_type = static_cast<ColorizerType>(module_type);

		const auto& required_attributes = ColorizerSpecifications::Specifications.at(colorizer_type).necessaryAttributes();
		return rasterizerFullfillsRequirements(required_attributes);
	}
	else if (module == RendererModule::Postprocessor)
	{
		const auto processor_type = static_cast<PostprocessorType>(module_type);
		const auto available_attributes = m_rasterizer->availableAttributes();

		const auto& required_attributes = PostprocessorSpecifications::Specifications.at(processor_type).necessaryAttributes();
		return rasterizerFullfillsRequirements(required_attributes);
	}

	return true; // all other modules are supported in any case for now
}

void Renderer::changeModule(RendererModule module, int module_type)
{
	switch (module)
	{
		case RendererModule::PointCloudProvider: // We don't allow manually changing the provider. It is tied to the file type loaded.
			break;
		case RendererModule::Rasterizer:
			changeRasterizer(static_cast<RasterizerType>(module_type));
			break;
		case RendererModule::Colorizer:
			changeColorizer(static_cast<ColorizerType>(module_type));
			break;
		default:
			throw std::runtime_error("Invalid module type");
			break;
	}

	emit modulesChanged();
}

void Renderer::changeRasterizer(RasterizerType rasterizer_type)
{
	assert(isModuleTypeSupported(RendererModule::Rasterizer, static_cast<int>(rasterizer_type)));

	if (m_rasterizer == nullptr || m_rasterizer->type() != rasterizer_type)
	{
		auto was_mtt_rasterizer = m_rasterizer != nullptr && m_rasterizer->type() == RasterizerType::MultiTemporalPointPrimitiveRasterizer;

		m_rasterizer = RasterizerSpecifications::Specifications.at(rasterizer_type).createRasterizer(m_opengl_context, m_pointcloud_provider, m_camera);
		if (rasterizer_type == RasterizerType::MultiTemporalPointPrimitiveRasterizer)
		{
			dynamic_cast<MultiTemporalPointCloudProvider*>(m_pointcloud_provider)->setProvideMultipleTimestamps(true);
			dynamic_cast<MultiTemporalPointPrimitiveRasterizer*>(m_rasterizer.get())->setFramebuffer(m_framebuffer);
		}
		else if (was_mtt_rasterizer)
		{
			dynamic_cast<MultiTemporalPointCloudProvider*>(m_pointcloud_provider)->setProvideMultipleTimestamps(false);
		}

		if (m_pointcloud_provider->hasAttribute(geometry::AttributeSemantic::Color))
		{
			changeColorizer(ColorizerType::AttributeBased);
		}
		else
		{
			changeColorizer(ColorizerType::SingleColor);
		}

		m_postprocessor_pipeline.removeIncompatiblePostprocessors(m_rasterizer->availableAttributes());

		connect(m_rasterizer.get(), &AbstractRasterizer::requiredAttributesChanged, this, &Renderer::reloadShader);
	}
}

void Renderer::changeColorizer(ColorizerType colorizer_type)
{
	assert(isModuleTypeSupported(RendererModule::Colorizer, static_cast<int>(colorizer_type)));

	auto m_colorizer = m_rasterizer->setColorizerType(colorizer_type);
	connect(m_colorizer, &AbstractColorizer::shaderRequiresRecompile, this, &Renderer::reloadShader);
	reloadShader();
}

void Renderer::appendPostprocessor(PostprocessorType postprocessor_type)
{
	assert(isModuleTypeSupported(RendererModule::Postprocessor, static_cast<int>(postprocessor_type)));

	m_postprocessor_pipeline.appendPostprocessor(PostprocessorSpecifications::Specifications.at(postprocessor_type).createPostprocessor(m_opengl_context, m_camera));
}

void Renderer::removePostprocessor(uint index)
{
	m_postprocessor_pipeline.removePostprocessor(index);
}

size_t Renderer::numberOfPostprocessors() const noexcept
{
	return m_postprocessor_pipeline.numberOfPostprocessors();
}

void Renderer::postprocessorInputsUpdated()
{
	m_framebuffer->setRequiredAttachments(m_postprocessor_pipeline.requiredFramebufferAttachments());
	reloadShader();
}

int Renderer::moduleType(RendererModule module) const
{
	switch (module)
	{
		case RendererModule::PointCloudProvider:
			return static_cast<int>(m_pointcloud_provider->type());
			break;
		case RendererModule::Rasterizer:
			return static_cast<int>(m_rasterizer->type());
			break;
		case RendererModule::Colorizer:
			return static_cast<int>(m_rasterizer->m_colorizer->type());
			break;
		default:
			throw std::invalid_argument("Invalid module type");
	}
}

std::vector<AbstractParameter*>& Renderer::moduleParameters(RendererModule module) const
{
	switch (module)
	{
		case RendererModule::PointCloudProvider:
			return m_pointcloud_provider->parameters();
			break;
		case RendererModule::Rasterizer:
			return m_rasterizer->parameters();
			break;
		case RendererModule::Colorizer:
			return m_rasterizer->m_colorizer->parameters();
			break;
		default:
			throw std::invalid_argument("Invalid module type");
	}
}

std::vector<AbstractParameter*>& Renderer::postprocessorParameters(uint index) const
{
	return m_postprocessor_pipeline.postprocessorParameters(index);
}

std::vector<AbstractParameter*>& Renderer::lastPostprocessorParameters() const
{
	return m_postprocessor_pipeline.postprocessorParameters(m_postprocessor_pipeline.numberOfPostprocessors() - 1);
}

}
