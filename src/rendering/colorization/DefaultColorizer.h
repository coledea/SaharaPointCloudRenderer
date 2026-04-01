#pragma once

#include "AbstractColorizer.h"

namespace sahara::rendering
{

class DefaultColorizer : public AbstractColorizer
{
public:
	DefaultColorizer() noexcept;

	~DefaultColorizer();

	void reloadShaderSpecificationsFromDisk() override;
	void setCompiledShaderProgram(QOpenGLShaderProgram* shader_program) override;

	virtual ColorizerType type() const noexcept override;

private:
};

}