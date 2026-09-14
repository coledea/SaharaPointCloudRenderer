#pragma once

#include "CSVColumn.h"
#include "geometry/AttributeSemantics.h"
#include "geometry/Color.h"
#include "geometry/StaticPointCloud.h"

#include <QString>
#include <QStringList>

namespace sahara::io
{

class CSVAttributeParser
{
public:
	static std::unique_ptr<CSVAttributeParser> floatParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column);
	static std::unique_ptr<CSVAttributeParser> intParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column);
	static std::unique_ptr<CSVAttributeParser> colorFromIntParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column_r, const CSVColumn& column_g, const CSVColumn& column_b);
	static std::unique_ptr<CSVAttributeParser> colorFromFloatParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column_r, const CSVColumn& column_g, const CSVColumn& column_b);
	static std::unique_ptr<CSVAttributeParser> vectorParser(geometry::AttributeSemantic attribute_semantic, const QString& name, const CSVColumn& column_x, const CSVColumn& column_y, const CSVColumn& column_z);

	virtual void parse(const QStringList& list) = 0;
	virtual void finishAndAddAttribute(geometry::StaticPointCloud* pointcloud) = 0;
};

}