#include "ShaderUtilities.h"

namespace sahara::utils
{

// These defines are used by colorizers/postprocessors to toggle the passthrough of vertex attributes to the fragment shader.
const std::unordered_map<geometry::AttributeSemantic, QString> ShaderStringsFactory::VERTEX_SHADER_OUTPUT_DEFINE_STRINGS = {
	{ geometry::AttributeSemantic::ID, "#define VERTEX_SHADER_OUTPUT_ID" },
	{ geometry::AttributeSemantic::Position, "#define VERTEX_SHADER_OUTPUT_POSITION" },
	{ geometry::AttributeSemantic::Color, "#define VERTEX_SHADER_OUTPUT_COLOR" },
	{ geometry::AttributeSemantic::Normal, "#define VERTEX_SHADER_OUTPUT_NORMAL" },
	{ geometry::AttributeSemantic::SegmentID, "#define VERTEX_SHADER_OUTPUT_SEGMENT" },
	{ geometry::AttributeSemantic::Custom0, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom1, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom2, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom3, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom4, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom5, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom6, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom7, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom8, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom9, "#define VERTEX_SHADER_OUTPUT_CUSTOM" },
	{ geometry::AttributeSemantic::Custom10, "#define VERTEX_SHADER_OUTPUT_CUSTOM" }
};

// These defines are used by postprocessors to toggle the write of vertex attributes to framebuffer attachments.
const std::unordered_map<geometry::AttributeSemantic, QString> ShaderStringsFactory::FRAGMENT_SHADER_OUTPUT_DEFINE_STRINGS = {
	{ geometry::AttributeSemantic::ID, "#define FRAGMENT_SHADER_OUTPUT_ID\n" },
	{ geometry::AttributeSemantic::Position, "#define FRAGMENT_SHADER_OUTPUT_POSITION\n" },
	//	{ geometry::AttributeSemantic::Color, "#define FRAGMENT_SHADER_OUTPUT_COLOR\n" },   // color is always written
	{ geometry::AttributeSemantic::Normal, "#define FRAGMENT_SHADER_OUTPUT_NORMAL\n" },
	{ geometry::AttributeSemantic::SegmentID, "#define FRAGMENT_SHADER_OUTPUT_SEGMENT\n" },
	{ geometry::AttributeSemantic::Custom0, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom1, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom2, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom3, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom4, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom5, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom6, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom7, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom8, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom9, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" },
	{ geometry::AttributeSemantic::Custom10, "#define FRAGMENT_SHADER_OUTPUT_CUSTOM\n" }
};

const std::unordered_map<geometry::AttributeType, QString> ShaderStringsFactory::ATTRIBUTE_TYPE_SUFFIXES = {
	{ geometry::AttributeType::Float, "_FLOAT" },
	{ geometry::AttributeType::Int, "_INT" },
	{ geometry::AttributeType::Uint, "_UINT" },
	{ geometry::AttributeType::Vector3D, "_VEC" },
	{ geometry::AttributeType::Color, "_VEC" }
};

QString ShaderStringsFactory::vertexShaderOutputDefineString(geometry::AttributeSemantic semantic, geometry::AttributeType type)
{
	QString result = VERTEX_SHADER_OUTPUT_DEFINE_STRINGS.at(semantic);
	if (isCustom(semantic))
	{
		result += ATTRIBUTE_TYPE_SUFFIXES.at(type);
	}
	result += "\n";
	return result;
}

QString ShaderStringsFactory::fragmentShaderOutputDefineString(geometry::AttributeSemantic semantic, geometry::AttributeType type)
{
	QString result = FRAGMENT_SHADER_OUTPUT_DEFINE_STRINGS.at(semantic);
	if (isCustom(semantic))
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

}