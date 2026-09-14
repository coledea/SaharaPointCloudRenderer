#include "CSVLoader.h"

#include "geometry/Color.h"
#include "io/CSVColumnSettings.h"
#include "utils/MemoryStream.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace sahara::io
{

CSVLoader::CSVLoader()
{
}

std::unique_ptr<QFile> CSVLoader::openFile(const QString& file_path)
{
	auto file = std::make_unique<QFile>(file_path);
	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return nullptr;
	}

	return file;
}

void CSVLoader::processFile(QFile* file)
{
	auto column_count = m_column_settings.m_columns.size();
	auto values = std::vector(column_count, std::vector<QString>());

	while (!file->atEnd())
	{
		QString line = file->readLine();
		QStringList list = line.split(',');

		for (auto& factory : m_attribute_parser)
		{
			factory->parse(list);
		}
	}
}

std::vector<CSVColumn> CSVLoader::tryGetColumns(const std::vector<CSVColumn::ColumnType>& types)
{
	std::vector<CSVColumn> requested_data;
	for (const auto& type : types)
	{
		if (!m_column_settings.columnAvailable(type))
		{
			return std::vector<CSVColumn>();
		}

		requested_data.push_back(m_column_settings.columnForType(type));
	}

	return requested_data;
}

void CSVLoader::addStaticAttributeParsers()
{
	auto coords = tryGetColumns({ CSVColumn::ColumnType::PositionX, CSVColumn::ColumnType::PositionY, CSVColumn::ColumnType::PositionZ });
	if (coords.size() == 3)
	{
		m_attribute_parser.push_back(CSVAttributeParser::vectorParser(geometry::AttributeSemantic::Position, "position", coords[0], coords[1], coords[2]));
	}

	auto normals = tryGetColumns({ CSVColumn::ColumnType::NormalX, CSVColumn::ColumnType::NormalY, CSVColumn::ColumnType::NormalZ });
	if (normals.size() == 3)
	{
		m_attribute_parser.push_back(CSVAttributeParser::vectorParser(geometry::AttributeSemantic::Normal, "normal", normals[0], normals[1], normals[2]));
	}

	auto colors = tryGetColumns({ CSVColumn::ColumnType::ColorR, CSVColumn::ColumnType::ColorG, CSVColumn::ColumnType::ColorB });
	if (colors.size() == 3)
	{
		if (colors[0].valueType() == CSVColumn::ValueType::INT && colors[1].valueType() == CSVColumn::ValueType::INT && colors[2].valueType() == CSVColumn::ValueType::INT)
		{
			m_attribute_parser.push_back(CSVAttributeParser::colorFromIntParser(geometry::AttributeSemantic::Color, "color", colors[0], colors[1], colors[2]));
		}
		else
		{
			m_attribute_parser.push_back(CSVAttributeParser::colorFromFloatParser(geometry::AttributeSemantic::Color, "color", colors[0], colors[1], colors[2]));
		}
	}

	auto segmentID = tryGetColumns({ CSVColumn::ColumnType::SegmentID });
	if (segmentID.size() == 1)
	{
		m_attribute_parser.push_back(CSVAttributeParser::intParser(geometry::AttributeSemantic::SegmentID, "segment_id", segmentID[0]));
	}
}

void CSVLoader::addCustomAttributeParsers()
{
	auto custom_attribute_slot = geometry::AttributeSemantic::Custom0;
	for (const auto& column : m_column_settings.m_columns)
	{
		if (custom_attribute_slot >= geometry::AttributeSemantic::Custom10)
		{
			std::cout << "We currently support only 11 custom attributes at the same time." << std::endl;
			break;
		}

		if (column.columnType() != CSVColumn::ColumnType::Custom)
		{
			continue;
		}

		switch (column.valueType())
		{
			case CSVColumn::ValueType::FLOAT:
				m_attribute_parser.push_back(CSVAttributeParser::floatParser(custom_attribute_slot, column.header(), column));
				break;
			case CSVColumn::ValueType::INT:
				m_attribute_parser.push_back(CSVAttributeParser::intParser(custom_attribute_slot, column.header(), column));
				break;
			default:
				break;
		}

		custom_attribute_slot = static_cast<geometry::AttributeSemantic>(static_cast<int>(custom_attribute_slot) + 1);
	}
}

void CSVLoader::addAttributes(geometry::StaticPointCloud* pointcloud)
{
	for (auto& factory : m_attribute_parser)
	{
		factory->finishAndAddAttribute(pointcloud);
	}
	m_attribute_parser.clear();
}

std::unique_ptr<geometry::StaticPointCloud> CSVLoader::load(const std::string& file_path, const CSVColumnSettings& column_settings)
{
	m_column_settings = column_settings;

	auto pointcloud = std::make_unique<geometry::StaticPointCloud>();

	try
	{
		auto file = openFile(QString::fromStdString(file_path));
		if (!file)
		{
			std::cerr << "Failed to open file: " << file_path << std::endl;
			return nullptr;
		}

		if (m_column_settings.m_header_exists == CSVColumnSettings::HeaderExistsSetting::Yes)
		{
			file->readLine(); // skip header
		}

		addStaticAttributeParsers();
		addCustomAttributeParsers();

		processFile(file.get());
		file->close();

		addAttributes(pointcloud.get());

		return pointcloud;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught exception during csv file load: " << e.what() << std::endl;
	}

	return nullptr;
}

}