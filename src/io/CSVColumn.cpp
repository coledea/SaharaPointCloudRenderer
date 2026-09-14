#include "CSVColumn.h"

namespace sahara::io
{

const std::map<CSVColumn::ColumnType, QString> CSVColumn::column_type_strings = {
	{ ColumnType::PositionX, "Pos X" },
	{ ColumnType::PositionY, "Pos Y" },
	{ ColumnType::PositionZ, "Pos Z" },
	{ ColumnType::ColorR, "Color R" },
	{ ColumnType::ColorG, "Color G" },
	{ ColumnType::ColorB, "Color B" },
	{ ColumnType::NormalX, "Normal X" },
	{ ColumnType::NormalY, "Normal Y" },
	{ ColumnType::NormalZ, "Normal Z" },
	{ ColumnType::SegmentID, "Segment ID" },
	{ ColumnType::Custom, "Custom" },
	{ ColumnType::None, "None" }
};

CSVColumn::CSVColumn(int index, const QString& header)
	: m_index(index)
	, m_header(header)
	, m_column_type(ColumnType::Custom)
	, m_value_type(ValueType::INVALID)
{
}

void CSVColumn::setColumnType(ColumnType type)
{
	m_column_type = type;
}

CSVColumn::ColumnType CSVColumn::columnType() const noexcept
{
	return m_column_type;
}

void CSVColumn::setValueType(ValueType type)
{
	m_value_type = type;
}

CSVColumn::ValueType CSVColumn::valueType() const noexcept
{
	return m_value_type;
}

void CSVColumn::setLimit(float min, float max)
{
	m_limits.min = min;
	m_limits.max = max;
}

void CSVColumn::updateLimit(float number)
{
	m_limits.min = std::min(m_limits.min, number);
	m_limits.max = std::max(m_limits.max, number);
}

CSVColumn::Limits CSVColumn::limits() const noexcept
{
	return m_limits;
}

QString CSVColumn::header() const noexcept
{
	return m_header;
}

int CSVColumn::index() const noexcept
{
	return m_index;
}

}