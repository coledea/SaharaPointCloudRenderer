#include "CSVColumnTypeDetection.h"

#include <filesystem>
#include <fstream>

namespace sahara::io
{

const std::vector<std::pair<QRegularExpression, CSVColumn::ColumnType>> CSVColumnTypeDetection::m_regular_expressions = {
	{ QRegularExpression("^x$|^pos(ition)?[\\s\\.]?x$|^x[\\s\\.]?pos(ition)?$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::PositionX },
	{ QRegularExpression("^y$|^pos(ition)?[\\s\\.]?y$|^y[\\s\\.]?pos(ition)?$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::PositionY },
	{ QRegularExpression("^z$|^pos(ition)?[\\s\\.]?z$|^z[\\s\\.]?pos(ition)?$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::PositionZ },
	{ QRegularExpression("^r$|^red$|color[\\s\\.]?x$|^color[\\s\\.]?r$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::ColorR },
	{ QRegularExpression("^g$|^green$|^color[\\s\\.]?y$|^color[\\s\\.]?g$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::ColorG },
	{ QRegularExpression("^b$|^blue$|^color[\\s\\.]?z$|^color[\\s\\.]?b$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::ColorB },
	{ QRegularExpression("^n(ormals?)?[\\s\\.[0-9]*]?x$|^x[\\s\\.[0-9]*]?n(ormals?)?$$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::NormalX },
	{ QRegularExpression("^n(ormals?)?[\\s\\.[0-9]*]?y$|^y[\\s\\.[0-9]*]?n(ormals?)?$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::NormalY },
	{ QRegularExpression("^n(ormals?)?[\\s\\.[0-9]*]?z$|^z[\\s\\.[0-9]*]?n(ormals?)?$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::NormalZ },
	{ QRegularExpression("^sem(antic)?[\\s\\.]?(class)?[\\s\\.]?id$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::SegmentID },
	{ QRegularExpression("^id$|^seg(ment)?[\\s\\.]?id$", QRegularExpression::CaseInsensitiveOption), CSVColumn::ColumnType::SegmentID }
};

void CSVColumnTypeDetection::detectColumnTypes(CSVColumnSettings& column_settings)
{
	ScoreTable score_table(column_settings.m_columns.size());

	scoreBasedOnHeader(column_settings.m_columns, score_table);
	scoreBasedOnPosition(column_settings.m_columns, score_table);
	scoreBasedOnLimits(column_settings.m_columns, score_table);
	scoreBasedOnNumberType(column_settings.m_columns, score_table);
	scoreBasedOnClusters(column_settings.m_columns, score_table);

	assignColumnTypes(column_settings, score_table);
}

void CSVColumnTypeDetection::assignColumnTypes(CSVColumnSettings& column_settings, ScoreTable& score_table)
{
	const int score_threshold = 50;

	for (int typeID = 0; typeID < static_cast<int>(CSVColumn::ColumnType::Custom); ++typeID)
	{
		auto type = static_cast<CSVColumn::ColumnType>(typeID);

		int best_column = -1;
		for (int col = 0; col < column_settings.m_columns.size(); col++)
		{
			if (score_table[col][type] > score_threshold)
			{
				if (best_column == -1 || score_table[col][type] > score_table[best_column][type])
				{
					best_column = col;
				}
			}
		}

		if (best_column != -1)
		{
			column_settings.setColumn(type, column_settings.m_columns[best_column]);
		}
	}
}

void CSVColumnTypeDetection::scoreBasedOnHeader(const std::vector<CSVColumn>& columns, ScoreTable& score_table)
{
	const int reward_score = 1000;

	for (const auto& column : columns)
	{
		for (const auto& regex : m_regular_expressions)
		{
			if (regex.first.match(column.header()).hasMatch())
			{
				score_table[column.index()][regex.second] += reward_score;
				break;
			}
		}
	}
}

void CSVColumnTypeDetection::scoreBasedOnNumberType(const std::vector<CSVColumn>& columns, ScoreTable& score_table)
{
	const int penalty_score = -100;
	const int reward_score = 20;

	for (int col = 0; col < columns.size(); ++col)
	{
		switch (columns[col].valueType())
		{
			case CSVColumn::ValueType::INT:
				score_table[col][CSVColumn::ColumnType::ColorR] += reward_score;
				score_table[col][CSVColumn::ColumnType::ColorG] += reward_score;
				score_table[col][CSVColumn::ColumnType::ColorB] += reward_score;
				score_table[col][CSVColumn::ColumnType::SegmentID] += reward_score;
				break;
			case CSVColumn::ValueType::FLOAT:
				score_table[col][CSVColumn::ColumnType::SegmentID] += penalty_score;
				break;
		}
	}
}

bool limitBetween(float min, float max, CSVColumn::Limits& limits)
{
	return limits.min >= min && limits.max <= max;
}

void CSVColumnTypeDetection::scoreBasedOnLimits(const std::vector<CSVColumn>& columns, ScoreTable& score_table)
{
	const int penalty_score = -10;
	const int reward_score = 50;

	for (int col = 0; col < columns.size(); col++)
	{
		auto limits = columns[col].limits();
		if (limitBetween(0.f, 1.f, limits))
		{
			score_table[col][CSVColumn::ColumnType::ColorR] += reward_score;
			score_table[col][CSVColumn::ColumnType::ColorG] += reward_score;
			score_table[col][CSVColumn::ColumnType::ColorB] += reward_score;
		}
		else if (limitBetween(0.f, 255.f, limits) && columns[col].valueType() == CSVColumn::ValueType::INT)
		{
			score_table[col][CSVColumn::ColumnType::ColorR] += reward_score;
			score_table[col][CSVColumn::ColumnType::ColorG] += reward_score;
			score_table[col][CSVColumn::ColumnType::ColorB] += reward_score;

			if (limitBetween(0.f, 10.f, limits))
			{
				score_table[col][CSVColumn::ColumnType::SegmentID] += reward_score;

				score_table[col][CSVColumn::ColumnType::ColorR] += penalty_score;
				score_table[col][CSVColumn::ColumnType::ColorG] += penalty_score;
				score_table[col][CSVColumn::ColumnType::ColorB] += penalty_score;
			}
		}
		else if (limitBetween(-1.f, 1.f, limits))
		{
			score_table[col][CSVColumn::ColumnType::NormalX] += reward_score;
			score_table[col][CSVColumn::ColumnType::NormalY] += reward_score;
			score_table[col][CSVColumn::ColumnType::NormalZ] += reward_score;
		}
	}
}

void CSVColumnTypeDetection::scoreBasedOnPosition(const std::vector<CSVColumn>& columns, ScoreTable& score_table)
{
	const int position_score = 100;
	const int rgb_score = 10;

	for (int col = 0; col < columns.size(); ++col)
	{
		auto& column_scores = score_table[col];
		switch (col)
		{
			case 0:
				column_scores[CSVColumn::ColumnType::PositionX] += position_score;
				break;
			case 1:
				column_scores[CSVColumn::ColumnType::PositionY] += position_score;
				break;
			case 2:
				column_scores[CSVColumn::ColumnType::PositionZ] += position_score;
				break;
			case 3:
				column_scores[CSVColumn::ColumnType::ColorR] += rgb_score;
				break;
			case 4:
				column_scores[CSVColumn::ColumnType::ColorG] += rgb_score;
				break;
			case 5:
				column_scores[CSVColumn::ColumnType::ColorB] += rgb_score;
				break;
		}
	}
}

float averageScoreOfRange(ScoreTable& score_table, CSVColumn::ColumnType column_type, Range& range)
{
	float sum = 0;
	for (int i = range.first; i <= range.second; i++)
	{
		sum += score_table[i][column_type];
	}

	return sum / (range.second - range.first + 1);
}

Range findHighestScoringRangeForType(ScoreTable& score_table, CSVColumn::ColumnType column_type, int score_threshold, int range_length)
{
	int end_column = 0;
	Range current_range = { 0, 0 };

	for (int start_column = 0; start_column < score_table.size(); start_column++)
	{
		int current_score = score_table[start_column][column_type];

		if (current_score >= score_threshold)
		{
			for (int column = start_column + 1; column < start_column + range_length && column < score_table.size() && score_table[column][column_type] >= score_threshold; column++)
			{
				end_column = column;
			}

			if ((end_column - start_column) + 1 >= range_length)
			{
				Range new_range = { start_column, end_column };
				if (averageScoreOfRange(score_table, column_type, new_range) > averageScoreOfRange(score_table, column_type, current_range))
				{
					current_range = new_range;
				}
			}
		}
	}

	return current_range;
}

void CSVColumnTypeDetection::scoreBasedOnClusters(const std::vector<CSVColumn>& columns, ScoreTable& score_table)
{
	const int cluster_score_threshold = 50;
	const int cluster_score = 20;
	const int cluster_size = 3;

	std::vector<CSVColumn::ColumnType> cluster_starting_types = { CSVColumn::ColumnType::ColorR, CSVColumn::ColumnType::NormalX };

	for (auto starting_type : cluster_starting_types)
	{
		Range best_range = findHighestScoringRangeForType(score_table, starting_type, cluster_score_threshold, cluster_size);

		if (best_range.first == best_range.second)
		{
			break;
		}

		for (int i = 0; i < cluster_size; i++)
		{
			score_table[best_range.first + i][static_cast<CSVColumn::ColumnType>(static_cast<int>(starting_type) + i)] += cluster_score;
		}
	}
}

}