#pragma once

#include "geometry/AttributeMetadata.h"
#include "rendering/OpenGLContext.h"
#include "rendering/RendererModuleTypes.h"
#include "rendering/parameters/AbstractParameter.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLShaderProgram>

namespace sahara::rendering
{

class AbstractColorizer : public QObject
{
	Q_OBJECT

public:
	AbstractColorizer(OpenGLContext* opengl_context) noexcept;

	virtual ~AbstractColorizer() = default;

	virtual void reloadShaderSpecificationsFromDisk() = 0;
	virtual void setCompiledShaderProgram(QOpenGLShaderProgram* shader_program) = 0;

	virtual ColorizerType type() const noexcept = 0;
	virtual std::set<geometry::AttributeSpecification> requestedAttributes() const = 0;
	const QString& finalShaderCode() const noexcept;
	std::vector<AbstractParameter*>& parameters() noexcept;

signals:
	void shaderRequiresRecompile();

protected:
	OpenGLContext* m_opengl_context;
	std::vector<AbstractParameter*> m_parameters;
	QString m_final_shader_code;
	QOpenGLShaderProgram* m_shader_program;
};

}