#pragma once

#include "rendering/Framebuffer.h"
#include "rendering/colorization/AbstractColorizer.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLShaderProgram>
#include <QRect>
#include <set>

namespace sahara::rendering
{

class AbstractRasterizer : public QObject
{
	Q_OBJECT

public:
	AbstractRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~AbstractRasterizer() = default;

	virtual void recompileShaders(const std::set<geometry::AttributeSpecification>& required_outputs) = 0;
	virtual void run(Framebuffer* framebuffer, int read_fbo_index) = 0;

	AbstractColorizer* setColorizerType(ColorizerType colorizer_type);
	std::vector<AbstractParameter*>& parameters() noexcept;

	std::unique_ptr<AbstractColorizer> m_colorizer;

	virtual RasterizerType type() const noexcept = 0;

	struct AnnotationViewport
	{
		QRect viewport;
		QMatrix4x4 view_projection_matrix;
	};

	virtual std::vector<AnnotationViewport> annotationViewports() const;

	virtual std::set<geometry::AttributeSpecification> generatableAttributes() noexcept; // Override if the rasterizer can generate attributes (maybe be based on the point cloud providers attributes).
	virtual std::set<geometry::AttributeSpecification> availableAttributes();
	virtual std::set<geometry::AttributeSpecification> requiredAttributes(const std::set<geometry::AttributeSpecification>& required_outputs) const;

signals:
	void requiredAttributesChanged();

protected:
	OpenGLContext* m_opengl_context;
	AbstractPointCloudProvider* m_pointcloud_provider;
	navigation::Camera* m_camera;
	std::vector<AbstractParameter*> m_parameters;
	std::set<geometry::AttributeSpecification> m_required_input_attributes;
};

}
