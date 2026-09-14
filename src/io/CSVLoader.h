#pragma once

#include "CSVColumn.h"
#include "geometry/StaticPointCloud.h"
#include "io/CSVAttributeParser.h"
#include "io/CSVColumnSettings.h"

#include <qfile.h>

namespace sahara::io
{
class CSVLoader
{
public:
	CSVLoader();
	std::unique_ptr<geometry::StaticPointCloud> load(const std::string& file_path, const CSVColumnSettings& column_settings);

private:
	CSVColumnSettings m_column_settings;
	std::vector<std::unique_ptr<CSVAttributeParser>> m_attribute_parser;

	std::unique_ptr<QFile> openFile(const QString& file_path);
	void processFile(QFile* file);
	std::vector<CSVColumn> tryGetColumns(const std::vector<CSVColumn::ColumnType>& types);
	void addStaticAttributeParsers();
	void addCustomAttributeParsers();

	void addAttributes(geometry::StaticPointCloud* pointcloud);
};

}
