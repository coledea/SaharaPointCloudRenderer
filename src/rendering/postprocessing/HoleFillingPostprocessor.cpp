#include "HoleFillingPostprocessor.h"

#include "utils/ShaderUtilities.h"

namespace sahara::rendering
{

HoleFillingPostprocessor::HoleFillingPostprocessor(OpenGLContext* opengl_context)
	: AbstractPostprocessor(opengl_context)
{
	reloadShader();
}

HoleFillingPostprocessor::~HoleFillingPostprocessor()
{
}

void HoleFillingPostprocessor::reloadShader()
{
	m_opengl_context->makeCurrent();
	m_shader_program.removeAllShaders();

	auto shader_string = utils::ShaderStringsFactory::readShaderFile("./data/shaders/postprocessing/HoleFilling.comp");

	if (!m_shader_program.addShaderFromSourceCode(QOpenGLShader::Compute, shader_string))
	{
		qDebug() << "Compute shader compilation error:" << m_shader_program.log();
	}

	if (!m_shader_program.link())
	{
		qDebug() << "Program linking error!" << m_shader_program.log();
	}
	m_opengl_context->doneCurrent();
}

void HoleFillingPostprocessor::run(Framebuffer* framebuffer, int read_fbo_index)
{
	GLuint dispatch_groups_x = std::ceil(static_cast<float>(framebuffer->width()) / 32.0);
	GLuint dispatch_groups_y = std::ceil(static_cast<float>(framebuffer->height()) / 32.0);

	m_shader_program.bind();
	m_opengl_context->gl()->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer->textureID(FramebufferAttachmentTypes::DEPTH, read_fbo_index));
	m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_shader_program.programId(), "u_depth"), 0);

	m_opengl_context->gl()->glBindImageTexture(0, framebuffer->textureID(FramebufferAttachmentTypes::COLOR, read_fbo_index), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	m_opengl_context->gl()->glBindImageTexture(1, framebuffer->textureID(FramebufferAttachmentTypes::COLOR, 1 - read_fbo_index), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	m_opengl_context->gl()->glDispatchCompute(dispatch_groups_x, dispatch_groups_y, 1);
	m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_shader_program.release();
}

PostprocessorType HoleFillingPostprocessor::type() const noexcept
{
	return PostprocessorType::HoleFilling;
}

}