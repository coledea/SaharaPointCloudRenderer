#include "CSVAttributeParser.h"

#include "TypedCSVAttributeParser.h"

namespace sahara::io
{

std::unique_ptr<CSVAttributeParser> CSVAttributeParser::floatParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column)
{
	return std::make_unique<TypedCSVAttributeParser<float>>(attribute_semantic, name, [=](const QStringList& list) -> float {
		return list[column.index()].trimmed().toFloat();
	});
}

std::unique_ptr<CSVAttributeParser> CSVAttributeParser::intParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column)
{
	return std::make_unique<TypedCSVAttributeParser<int>>(attribute_semantic, name, [=](const QStringList& list) -> int {
		return list[column.index()].trimmed().toInt();
	});
}

std::unique_ptr<CSVAttributeParser> CSVAttributeParser::colorFromFloatParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column_r, const CSVColumn& column_g, const CSVColumn& column_b)
{
	return std::make_unique<TypedCSVAttributeParser<geometry::Color>>(attribute_semantic, name, [=](const QStringList& list) -> geometry::Color {
		auto data_r = list[column_r.index()].trimmed().toFloat();
		auto data_g = list[column_g.index()].trimmed().toFloat();
		auto data_b = list[column_b.index()].trimmed().toFloat();

		return geometry::Color(static_cast<int>(data_r * 255.0), static_cast<int>(data_g * 255.0), static_cast<int>(data_b * 255.0));
	});
}

std::unique_ptr<CSVAttributeParser> CSVAttributeParser::colorFromIntParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column_r, const CSVColumn& column_g, const CSVColumn& column_b)
{
	return std::make_unique<TypedCSVAttributeParser<geometry::Color>>(attribute_semantic, name, [=](const QStringList& list) -> geometry::Color {
		auto data_r = list[column_r.index()].trimmed().toInt();
		auto data_g = list[column_g.index()].trimmed().toInt();
		auto data_b = list[column_b.index()].trimmed().toInt();

		return geometry::Color(data_r, data_g, data_b);
	});
}

std::unique_ptr<CSVAttributeParser> CSVAttributeParser::vectorParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column_x, const CSVColumn& column_y, const CSVColumn& column_z)
{
	return std::make_unique<TypedCSVAttributeParser<QVector3D>>(attribute_semantic, name, [=](const QStringList& list) -> QVector3D {
		auto data_x = list[column_x.index()].trimmed().toFloat();
		auto data_y = list[column_y.index()].trimmed().toFloat();
		auto data_z = list[column_z.index()].trimmed().toFloat();

		return QVector3D(data_x, data_y, data_z);
	});
}

}
