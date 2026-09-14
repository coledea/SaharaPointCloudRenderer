#include "CSVColumnSettings.h"

namespace sahara::io
{

const CSVColumnSettings CSVColumnSettings::empty = CSVColumnSettings();

void CSVColumnSettings::setColumn(CSVColumn::ColumnType type, CSVColumn& column)
{
	if (columnAvailable(type) && type != CSVColumn::ColumnType::None && type != CSVColumn::ColumnType::Custom)
	{
		auto old_column = m_columns[m_type_column_map.at(type)];
		old_column.setColumnType(CSVColumn::ColumnType::None);
	}

	auto old_type = column.columnType();
	if (columnAvailable(old_type))
	{
		m_type_column_map.erase(old_type);
	}

	m_type_column_map[type] = column.index();
	column.setColumnType(type);
}

bool CSVColumnSettings::columnAvailable(CSVColumn::ColumnType type) const noexcept
{
	return m_type_column_map.find(type) != m_type_column_map.end();
}

CSVColumn CSVColumnSettings::columnForType(CSVColumn::ColumnType type) const
{
	return m_columns[m_type_column_map.at(type)];
}

};
