#include "TypedCSVAttributeParser.h"

namespace sahara::io
{

template <typename T>
TypedCSVAttributeParser<T>::TypedCSVAttributeParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const std::function<T(const QStringList&)> parser_function)
	: m_attribute_semantic(attribute_semantic)
	, m_name(name)
	, m_parser_function(parser_function)
{
}

template <typename T>
void TypedCSVAttributeParser<T>::finishAndAddAttribute(geometry::StaticPointCloud* pointcloud)
{
	geometry::AttributeDataVector<T> raw_data;
	auto data = std::move(m_data);
	raw_data.assign(data.data(), data.data() + data.size());
	auto attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<T>>(m_name, m_attribute_semantic, raw_data);
	auto attribute_data = std::make_unique<geometry::AttributeData<T>>(std::move(raw_data));
	pointcloud->addAttribute(std::move(attribute_data), std::move(attribute_metadata));
}

template <typename T>
void TypedCSVAttributeParser<T>::parse(const QStringList& list)
{
	m_data.push_back(m_parser_function(list));
}

}
