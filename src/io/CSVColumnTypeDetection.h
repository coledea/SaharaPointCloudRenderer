#pragma once

#include "CSVColumn.h"
#include "CSVColumnSettings.h"

#include <QRegularExpression>
#include <any>
#include <memory>
#include <string>
#include <vector>

namespace sahara::io
{
using ScoreTable = std::vector<std::map<CSVColumn::ColumnType, int>>;
using Range = std::pair<int, int>;

class CSVColumnTypeDetection
{
public:
	static void detectColumnTypes(CSVColumnSettings& column_settings);

private:
	static void assignColumnTypes(CSVColumnSettings& column_settings, ScoreTable& score_table);

	static void scoreBasedOnHeader(const std::vector<CSVColumn>& columns, ScoreTable& score_table);
	static void scoreBasedOnNumberType(const std::vector<CSVColumn>& columns, ScoreTable& score_table);
	static void scoreBasedOnLimits(const std::vector<CSVColumn>& columns, ScoreTable& score_table);
	static void scoreBasedOnPosition(const std::vector<CSVColumn>& columns, ScoreTable& score_table);
	static void scoreBasedOnClusters(const std::vector<CSVColumn>& columns, ScoreTable& score_table);

	static const std::vector<std::pair<QRegularExpression, CSVColumn::ColumnType>> m_regular_expressions;
};

}