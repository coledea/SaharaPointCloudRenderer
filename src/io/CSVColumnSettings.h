#pragma once

#include "CSVColumn.h"

#include <map>
#include <vector>

namespace sahara::io
{

class CSVColumnSettings
{

public:
	static const CSVColumnSettings empty;

	enum HeaderExistsSetting
	{
		No = 0,
		Yes = 1,
		Auto = 2
	};

	HeaderExistsSetting m_header_exists = HeaderExistsSetting::Auto;
	std::vector<CSVColumn> m_columns;

	void setColumn(CSVColumn::ColumnType type, CSVColumn& column);
	bool columnAvailable(CSVColumn::ColumnType type) const noexcept;
	CSVColumn columnForType(CSVColumn::ColumnType type) const;

private:
	std::map<CSVColumn::ColumnType, int> m_type_column_map;
};

};
