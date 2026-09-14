#include "EyeDomeLightingPostprocessor.h"

#include "utils/ShaderUtilities.h"

namespace sahara::rendering
{

EyeDomeLightingPostprocessor::EyeDomeLightingPostprocessor(OpenGLContext* opengl_context, navigation::Camera* camera)
	: AbstractPostprocessor(opengl_context)
	, m_camera(camera)
{
	m_strength_parameter = std::make_unique<RangeParameter<float>>("Strength", 1.0, 0.0, 10.0, 0.001);
	connect(m_strength_parameter.get(), &RangeParameter<float>::valueChanged, this, &EyeDomeLightingPostprocessor::onStrengthChanged);
	m_parameters.push_back(m_strength_parameter.get());

	connect(m_camera, &navigation::Camera::farPlaneChanged, this, &EyeDomeLightingPostprocessor::onCameraFarPlaneChanged);
	connect(m_camera, &navigation::Camera::nearPlaneChanged, this, &EyeDomeLightingPostprocessor::onCameraNearPlaneChanged);

	reloadShader();
}

EyeDomeLightingPostprocessor::~EyeDomeLightingPostprocessor()
{
}

void EyeDomeLightingPostprocessor::reloadShader()
{
	m_opengl_context->makeCurrent();
	m_shader_program.removeAllShaders();

	if (!m_shader_program.addShaderFromSourceCode(QOpenGLShader::Compute, utils::ShaderStringsFactory::readShaderFile("./data/shaders/postprocessing/EDL.comp")))
	{
		qDebug() << "Compute shader compilation error:" << m_shader_program.log();
	}

	if (!m_shader_program.link())
	{
		qDebug() << "Program linking error!" << m_shader_program.log();
	}

	m_shader_program.bind();
	m_shader_program.setUniformValue("u_strength", m_strength_parameter->value());
	m_shader_program.setUniformValue("u_far_plane", m_camera->cameraSpecifications().far_plane);
	m_shader_program.setUniformValue("u_near_plane", m_camera->cameraSpecifications().near_plane);
	m_shader_program.release();
	m_opengl_context->doneCurrent();
}

void EyeDomeLightingPostprocessor::run(Framebuffer* framebuffer, int read_fbo_index)
{
	GLuint dispatch_groups_x = std::ceil(static_cast<float>(framebuffer->width()) / 32.0);
	GLuint dispatch_groups_y = std::ceil(static_cast<float>(framebuffer->height()) / 32.0);

	m_shader_program.bind();
	m_opengl_context->gl()->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer->textureID(FramebufferAttachmentTypes::DEPTH, read_fbo_index));
	m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_shader_program.programId(), "u_input_depth_texture"), 0);

	m_opengl_context->gl()->glBindImageTexture(0, framebuffer->textureID(FramebufferAttachmentTypes::COLOR, read_fbo_index), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	m_opengl_context->gl()->glBindImageTexture(1, framebuffer->textureID(FramebufferAttachmentTypes::COLOR, 1 - read_fbo_index), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	m_opengl_context->gl()->glDispatchCompute(dispatch_groups_x, dispatch_groups_y, 1);
	m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_shader_program.release();
}

PostprocessorType EyeDomeLightingPostprocessor::type() const noexcept
{
	return PostprocessorType::EyeDomeLighting;
}

std::set<geometry::AttributeSpecification> EyeDomeLightingPostprocessor::necessaryAttributes()
{
	return {};
}

void EyeDomeLightingPostprocessor::onStrengthChanged()
{
	utils::setUniformValue(m_opengl_context, m_shader_program, "u_strength", m_strength_parameter->value());
}

void EyeDomeLightingPostprocessor::onCameraFarPlaneChanged(float far_plane)
{
	utils::setUniformValue(m_opengl_context, m_shader_program, "u_far_plane", far_plane);
}

void EyeDomeLightingPostprocessor::onCameraNearPlaneChanged(float near_plane)
{
	utils::setUniformValue(m_opengl_context, m_shader_program, "u_near_plane", near_plane);
}

}