#pragma once

#include "AbstractPostprocessor.h"
#include "navigation/Camera.h"
#include "rendering/parameters/RangeParameter.h"

#include <QOpenGLShaderProgram>

namespace sahara::rendering
{

class HoleFillingPostprocessor : public QObject, public AbstractPostprocessor
{
	Q_OBJECT

public:
	HoleFillingPostprocessor(OpenGLContext* opengl_context);
	~HoleFillingPostprocessor();

	void reloadShader() override;
	void run(Framebuffer* framebuffer, int read_fbo_index) override;
	PostprocessorType type() const noexcept override;

private:
	QOpenGLShaderProgram m_shader_program;
};

}