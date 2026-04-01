#pragma once

#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLShaderProgram>
#include <set>

namespace sahara::rendering
{

struct RasterizerShaderSpecifications
{
	QString vertex_shader_path;
	QString fragment_shader_path;
	std::set<geometry::AttributeSpecification> vertex_shader_inputs;
};

class AbstractRasterizer
{
public:
	AbstractRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~AbstractRasterizer() = default;

	virtual void reloadShaderSpecificationsFromDisk() = 0;
	virtual void setCompiledShaderProgram(QOpenGLShaderProgram* shader_program, const std::set<geometry::AttributeSpecification>& input_attributes) = 0;
	virtual void run() = 0;

	virtual RasterizerType type() const noexcept = 0;
	const RasterizerShaderSpecifications& shaderSpecifications() const noexcept;

	std::vector<AbstractParameter*>& parameters() noexcept;

protected:
	OpenGLContext* m_opengl_context;
	AbstractPointCloudProvider* m_pointcloud_provider;
	navigation::Camera* m_camera;
	std::vector<AbstractParameter*> m_parameters;
	QOpenGLShaderProgram* m_shader_program;
	std::set<geometry::AttributeSpecification> m_required_input_attributes;
	RasterizerShaderSpecifications m_shader_specifications;
};

}