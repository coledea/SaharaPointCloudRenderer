#pragma once

#include "AbstractColorizer.h"
#include "rendering/parameters/RangeParameter.h"

namespace sahara::rendering
{

class SingleColorColorizer : public AbstractColorizer
{
	Q_OBJECT

public:
	SingleColorColorizer(OpenGLContext* opengl_context) noexcept;

	~SingleColorColorizer();

	void reloadShaderSpecificationsFromDisk() override;
	void setCompiledShaderProgram(QOpenGLShaderProgram* shader_program) override;

	virtual ColorizerType type() const noexcept override;

private:
	std::unique_ptr<Parameter<QColor>> m_color_parameter;
	std::unique_ptr<RangeParameter<float>> m_original_color_factor_parameter;

	void onColorChanged();
	void onOriginalColorFactorChanged();
};

}