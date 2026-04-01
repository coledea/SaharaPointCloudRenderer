#include "Renderer.h"

#include "rendering/colorization/ColorizerFactory.h"
#include "rendering/postprocessing/PostprocessorFactory.h"
#include "rendering/rasterization/RasterizerFactory.h"

namespace sahara::rendering
{

Renderer::Renderer(AbstractPointCloudProvider* pointcloud_provider, OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera) noexcept
	: m_pointcloud_provider(pointcloud_provider)
	, m_opengl_context(opengl_context)
	, m_framebuffer(framebuffer)
	, m_camera(camera)
	, m_rasterizer(RasterizerFactory::createRasterizer((pointcloud_provider->type() == PointCloudProviderType::OOCSinglePointCloudProvider || pointcloud_provider->type() == PointCloudProviderType::OOCMultiTemporalPointCloudProvider) ? RasterizerType::OOCPointPrimitiveRasterizer : RasterizerType::PointPrimitiveRasterizer, opengl_context, pointcloud_provider, camera))
	, m_colorizer(ColorizerFactory::createColorizer(ColorizerType::None, opengl_context, pointcloud_provider, camera))
	, m_shader_program_factory(opengl_context)
	, m_shader_program(nullptr)
{
	reloadShader();
	connect(m_colorizer.get(), &AbstractColorizer::shaderRequiresRecompile, this, &Renderer::recompileShader);
}

void Renderer::render()
{
	m_pointcloud_provider->update();
	m_rasterizer->run();
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
	m_rasterizer->reloadShaderSpecificationsFromDisk();
	m_colorizer->reloadShaderSpecificationsFromDisk();
	recompileShader();
	m_postprocessor_pipeline.reloadShaders();
}

void Renderer::recompileShader()
{
	m_shader_program = m_shader_program_factory.createShaderProgram(m_rasterizer.get(), m_colorizer.get(), m_postprocessor_pipeline.shaderSpecifications());

	std::set<geometry::AttributeSpecification> vertex_shader_inputs;
	for (auto attribute : m_rasterizer->shaderSpecifications().vertex_shader_inputs)
	{
		vertex_shader_inputs.insert(attribute);
	}
	for (auto attribute : m_colorizer->shaderSpecifications().vertex_shader_outputs)
	{
		vertex_shader_inputs.insert(attribute);
	}
	for (auto attribute : m_postprocessor_pipeline.shaderSpecifications().required_vertex_attributes)
	{
		vertex_shader_inputs.insert(attribute);
	}

	vertex_shader_inputs.insert({ geometry::AttributeType::Vector3D, geometry::AttributeSemantic::Position }); // Position is always required
	m_pointcloud_provider->setRequiredAttributes(vertex_shader_inputs);										   // this creates the required GPU buffers. We need to call it before informing the rasterizer.
	m_rasterizer->setCompiledShaderProgram(m_shader_program.get(), vertex_shader_inputs);
	m_colorizer->setCompiledShaderProgram(m_shader_program.get());
}

bool Renderer::isModuleTypeSupported(RendererModule module, int module_type)
{
	if (module == RendererModule::Rasterizer)
	{
		const auto supported_rasterizers = m_pointcloud_provider->supportedRasterizers();
		return std::find(supported_rasterizers.begin(), supported_rasterizers.end(), static_cast<RasterizerType>(module_type)) != supported_rasterizers.end();
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
}

void Renderer::changeRasterizer(RasterizerType rasterizer_type)
{
	assert(isModuleTypeSupported(RendererModule::Rasterizer, static_cast<int>(rasterizer_type)));
	if (m_rasterizer->type() != rasterizer_type)
	{
		m_rasterizer = RasterizerFactory::createRasterizer(rasterizer_type, m_opengl_context, m_pointcloud_provider, m_camera);
		reloadShader();
	}
}

void Renderer::changeColorizer(ColorizerType colorizer_type)
{
	assert(isModuleTypeSupported(RendererModule::Colorizer, static_cast<int>(colorizer_type)));

	if (m_colorizer->type() != colorizer_type)
	{
		m_colorizer = ColorizerFactory::createColorizer(colorizer_type, m_opengl_context, m_pointcloud_provider, m_camera);
		reloadShader();
		connect(m_colorizer.get(), &AbstractColorizer::shaderRequiresRecompile, this, &Renderer::recompileShader);
	}
}

void Renderer::appendPostprocessor(PostprocessorType postprocessor_type)
{
	assert(isModuleTypeSupported(RendererModule::Postprocessor, static_cast<int>(postprocessor_type)));

	m_postprocessor_pipeline.appendPostprocessor(PostprocessorFactory::createPostprocessor(postprocessor_type, m_opengl_context, m_camera));
	if (m_postprocessor_pipeline.shaderSpecificationsChanged())
	{
		m_framebuffer->setRequiredAttachments(m_postprocessor_pipeline.shaderSpecifications().required_framebuffer_attachments);
		reloadShader();
	}
}

void Renderer::removePostprocessor(uint index)
{
	m_postprocessor_pipeline.removePostprocessor(index);
	if (m_postprocessor_pipeline.shaderSpecificationsChanged())
	{
		m_framebuffer->setRequiredAttachments(m_postprocessor_pipeline.shaderSpecifications().required_framebuffer_attachments);
		reloadShader();
	}
}

size_t Renderer::numberOfPostprocessors() const noexcept
{
	return m_postprocessor_pipeline.numberOfPostprocessors();
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
			return static_cast<int>(m_colorizer->type());
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
			return m_colorizer->parameters();
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