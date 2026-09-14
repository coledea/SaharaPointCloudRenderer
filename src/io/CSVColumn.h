#pragma once

#include <QString>
#include <any>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace sahara::io
{
class CSVColumn
{

public:
	struct Limits
	{
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::lowest();
	};

	enum class ColumnType : int
	{
		PositionX,
		PositionY,
		PositionZ,
		ColorR,
		ColorG,
		ColorB,
		NormalX,
		NormalY,
		NormalZ,
		SegmentID,
		Custom,
		None,
		_SIZE
	};

	static const std::map<ColumnType, QString> column_type_strings;

	enum class ValueType : int
	{
		INVALID,
		INT,
		FLOAT
	};

	CSVColumn(int index, const QString& header);

	void setColumnType(ColumnType type);
	ColumnType columnType() const noexcept;

	void setValueType(ValueType type);
	ValueType valueType() const noexcept;

	void setLimit(float min, float max);
	void updateLimit(float number);
	Limits limits() const noexcept;

	template <class T>
	std::vector<T> data() const;

	QString header() const noexcept;

	int index() const noexcept;

private:
	int m_index;
	QString m_header;
	ColumnType m_column_type;
	ValueType m_value_type;

	Limits m_limits;
};

}