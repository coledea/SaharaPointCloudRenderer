#include "ShaderUtilities.h"

namespace sahara::utils
{

// These defines are used by colorizers/postprocessors to toggle the passthrough of vertex attributes to the fragment shader.
const std::unordered_map<geometry::AttributeSemantic, QString> ShaderStringsFactory::ATTRIBUTE_INPUT_DEFINE_STRINGS = {
	{ geometry::AttributeSemantic::ID, "#define ATTRIBUTE_INPUT_ID" },
	{ geometry::AttributeSemantic::Position, "#define ATTRIBUTE_INPUT_POSITION" },
	{ geometry::AttributeSemantic::Color, "#define ATTRIBUTE_INPUT_COLOR" },
	{ geometry::AttributeSemantic::Normal, "#define ATTRIBUTE_INPUT_NORMAL" },
	{ geometry::AttributeSemantic::SegmentID, "#define ATTRIBUTE_INPUT_SEGMENT" },
	{ geometry::AttributeSemantic::Custom0, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom1, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom2, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom3, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom4, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom5, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom6, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom7, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom8, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom9, "#define ATTRIBUTE_INPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom10, "#define ATTRIBUTE_INPUT_CUSTOM" }
};

// These defines are used by postprocessors to toggle the write of vertex attributes to framebuffer attachments.
const std::unordered_map<geometry::AttributeSemantic, QString> ShaderStringsFactory::ATTRIBUTE_OUTPUT_DEFINE_STRINGS = {
	{ geometry::AttributeSemantic::ID, "#define ATTRIBUTE_OUTPUT_ID" },
	{ geometry::AttributeSemantic::Position, "#define ATTRIBUTE_OUTPUT_POSITION" },
	//	{ geometry::AttributeSemantic::Color, "#define ATTRIBUTE_OUTPUT_COLOR" },   // color is always written
	{ geometry::AttributeSemantic::Normal, "#define ATTRIBUTE_OUTPUT_NORMAL" },
	{ geometry::AttributeSemantic::SegmentID, "#define ATTRIBUTE_OUTPUT_SEGMENT" },
	{ geometry::AttributeSemantic::Custom0, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom1, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom2, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom3, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom4, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom5, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom6, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom7, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom8, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom9, "#define ATTRIBUTE_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom10, "#define ATTRIBUTE_OUTPUT_CUSTOM" }
};

const std::unordered_map<geometry::AttributeType, QString> ShaderStringsFactory::ATTRIBUTE_TYPE_SUFFIXES = {
	{ geometry::AttributeType::Float, "_FLOAT" },
	{ geometry::AttributeType::Int, "_INT" },
	{ geometry::AttributeType::Uint, "_UINT" },
	{ geometry::AttributeType::Vector3D, "_VEC" },
	{ geometry::AttributeType::Color, "_VEC" }
};

QString ShaderStringsFactory::attributeInputDefineString(geometry::AttributeSemantic semantic, geometry::AttributeType type)
{
	QString result = ATTRIBUTE_INPUT_DEFINE_STRINGS.at(semantic);
	if (attributeIsCustom(semantic))
	{
		result += ATTRIBUTE_TYPE_SUFFIXES.at(type);
	}
	result += "\n";
	return result;
}

QString ShaderStringsFactory::attributeOutputDefineString(geometry::AttributeSemantic semantic, geometry::AttributeType type)
{
	QString result = ATTRIBUTE_OUTPUT_DEFINE_STRINGS.at(semantic);
	if (attributeIsCustom(semantic))
	{
		result += ATTRIBUTE_TYPE_SUFFIXES.at(type);
	}
	result += "\n";
	return result;
}

std::string ShaderStringsFactory::vertexShaderInputName(geometry::AttributeSemantic attribute)
{
	switch (attribute)
	{
		case geometry::AttributeSemantic::Color:
			return "a_color";
			break;
		case geometry::AttributeSemantic::Position:
			return "a_position";
			break;

		case geometry::AttributeSemantic::ID:
			return "a_id";
			break;

		case geometry::AttributeSemantic::SegmentID:
			return "a_segment";
			break;

		case geometry::AttributeSemantic::Normal:
			return "a_normal";
			break;
		default:
			return "a_custom";
			break;
	}
}

QString ShaderStringsFactory::readShaderFile(const QString& file_path)
{
	QFile shader_file(file_path);
	if (!shader_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Could not open shader file:" << file_path;
		return QString();
	}
	return shader_file.readAll();
}

void ShaderStringsFactory::compileVertexFragmentShader(QOpenGLShaderProgram* shader_program, const QString& vertex_shader_path, const QString& fragment_shader_path)
{
	shader_program->removeAllShaders();

	if (!shader_program->addShaderFromSourceCode(QOpenGLShader::Vertex, readShaderFile(vertex_shader_path)))
	{
		qDebug() << "Vertex shader compilation error:" << shader_program->log();
	}

	if (!shader_program->addShaderFromSourceCode(QOpenGLShader::Fragment, readShaderFile(fragment_shader_path)))
	{
		qDebug() << "Fragment shader compilation error:" << shader_program->log();
	}

	if (!shader_program->link())
	{
		qDebug() << "Program linking error!" << shader_program->log();
	}
}

}