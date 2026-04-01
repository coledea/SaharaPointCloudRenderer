#pragma once

#include "geometry/AttributeSpecification.h"
#include "rendering/OpenGLContext.h"

#include <QFile>
#include <QOpenGLShaderProgram>

namespace sahara::utils
{

class ShaderStringsFactory
{
public:
	static QString vertexShaderOutputDefineString(geometry::AttributeSemantic semantic, geometry::AttributeType type);
	static QString fragmentShaderOutputDefineString(geometry::AttributeSemantic semantic, geometry::AttributeType type);
	static std::string vertexShaderInputName(geometry::AttributeSemantic attribute);

	static QString readShaderFile(const QString& file_path);

	static const std::unordered_map<geometry::AttributeType, QString> ATTRIBUTE_TYPE_SUFFIXES;

private:
	static const std::unordered_map<geometry::AttributeSemantic, QString> VERTEX_SHADER_OUTPUT_DEFINE_STRINGS;
	static const std::unordered_map<geometry::AttributeSemantic, QString> FRAGMENT_SHADER_OUTPUT_DEFINE_STRINGS;
};

inline const std::unordered_map<geometry::AttributeType, GLenum> ATTRIBUTE_GL_TYPES = {
	{ geometry::AttributeType::Uint, GL_UNSIGNED_INT },
	{ geometry::AttributeType::Float, GL_FLOAT },
	{ geometry::AttributeType::Int, GL_INT },
	{ geometry::AttributeType::Vector3D, GL_FLOAT },
	{ geometry::AttributeType::Color, GL_UNSIGNED_BYTE }
};

template <typename T>
void setUniformValue(rendering::OpenGLContext* context, QOpenGLShaderProgram& program, const char* name, T value)
{
	context->makeCurrent();
	program.bind();
	program.setUniformValue(name, value);
	program.release();
	context->doneCurrent();
}

// QOpenGLShaderProgram does not support passing of unsigned int uniform values (they are internally casted to int). So we have to do it via raw OpenGL in this case.
template <>
inline void setUniformValue<unsigned int>(rendering::OpenGLContext* context, QOpenGLShaderProgram& program, const char* name, unsigned int value)
{
	context->makeCurrent();
	program.bind();
	context->gl()->glUniform1ui(program.uniformLocation(name), value);
	program.release();
	context->doneCurrent();
}

}