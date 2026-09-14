#pragma once

#include "CSVAttributeParser.h"

namespace sahara::io
{

class CSVAttributeParser;

template <typename T>
class TypedCSVAttributeParser : public CSVAttributeParser
{
public:
	TypedCSVAttributeParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const std::function<T(const QStringList&)> parser_function);

	void parse(const QStringList& list) override;
	void finishAndAddAttribute(geometry::StaticPointCloud* pointcloud) override;

private:
	geometry::AttributeSemantic m_attribute_semantic;
	QString m_name;

	std::function<T(const QStringList&)> m_parser_function;
	std::vector<T> m_data;
};
}

#include "TypedCSVAttributeParser.inl"