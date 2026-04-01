#pragma once

#include "AbstractPostprocessor.h"
#include "navigation/Camera.h"
#include "rendering/parameters/RangeParameter.h"

#include <QOpenGLShaderProgram>

namespace sahara::rendering
{

class EyeDomeLightingPostprocessor : public QObject, public AbstractPostprocessor
{
	Q_OBJECT

public:
	EyeDomeLightingPostprocessor(OpenGLContext* opengl_context, navigation::Camera* camera);
	~EyeDomeLightingPostprocessor();

	void reloadShader() override;
	void run(Framebuffer* framebuffer, int read_fbo_index) override;
	PostprocessorType type() const noexcept override;

private:
	navigation::Camera* m_camera;
	QOpenGLShaderProgram m_shader_program;
	std::unique_ptr<RangeParameter<float>> m_strength_parameter;

	void onStrengthChanged();
	void onCameraFarPlaneChanged(float far_plane);
	void onCameraNearPlaneChanged(float near_plane);
};

}