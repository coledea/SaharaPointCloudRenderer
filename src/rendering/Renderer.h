#pragma once

#include "Framebuffer.h"
#include "RendererModuleTypes.h"
#include "navigation/Camera.h"
#include "rendering/colorization/AbstractColorizer.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"
#include "rendering/postprocessing/PostprocessorPipeline.h"
#include "rendering/rasterization/AbstractRasterizer.h"

namespace sahara::rendering
{

class Renderer : public QObject
{
	Q_OBJECT

public:
	Renderer(AbstractPointCloudProvider* pointcloud_provider, OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera) noexcept;

	void render();

	void reloadShader();

	bool isModuleTypeSupported(RendererModule module, int module_type);
	void changeModule(RendererModule module, int module_type);
	void changeRasterizer(RasterizerType rasterizer_type);
	void changeColorizer(ColorizerType colorizer_type);

	void appendPostprocessor(PostprocessorType postprocessor_type);
	void removePostprocessor(uint index);
	size_t numberOfPostprocessors() const noexcept;

	std::vector<AbstractParameter*>& moduleParameters(RendererModule module) const;
	std::vector<AbstractParameter*>& postprocessorParameters(uint index) const;
	std::vector<AbstractParameter*>& lastPostprocessorParameters() const;

	int moduleType(RendererModule module) const;
	std::vector<AbstractRasterizer::AnnotationViewport> annotationViewports() const;

signals:
	void modulesChanged();
	void postprocessorRemoved(int index);

protected:
	OpenGLContext* m_opengl_context;
	Framebuffer* m_framebuffer;
	navigation::Camera* m_camera;
	AbstractPointCloudProvider* m_pointcloud_provider;

	std::unique_ptr<AbstractRasterizer> m_rasterizer;
	PostprocessorPipeline m_postprocessor_pipeline;

private:
	bool rasterizerFullfillsRequirements(std::set<geometry::AttributeSpecification> requirements);
	void postprocessorInputsUpdated();
};

}
