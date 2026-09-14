#pragma once

#include "CSVColumn.h"
#include "CSVColumnSettings.h"
#include "CSVLoader.h"

#include <QComboBox>
#include <QDialog>
#include <QRegularExpression>

namespace Ui
{
class CSVLoadingDialog;
}

namespace sahara::io
{

class CSVLoadingDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CSVLoadingDialog(QString& file_name, const QString& delimiter = ",", QWidget* parent = nullptr);
	~CSVLoadingDialog();
	void populateTable(const QString& delimiter);

	const CSVColumnSettings& columnSettings();

private:
	static const QRegularExpression m_header_regex;

	void populateRows(QTextStream& in, const QString& delimiter);

	void configureTable();

	bool checkAndSetHeader(QString& first_line, const QString& delimiter);
	void setHeader(QString& header_line, const QString& delimiter);
	void addDropdowns(int row);
	void addRow(int row, QString& row_line, const QString& delimiter);

	void updateDropdownSelection(CSVColumn& column, CSVColumn::ColumnType new_type);

	Ui::CSVLoadingDialog* m_ui;
	QString m_file_name;

	std::vector<QString> m_header_names;
	std::vector<QComboBox*> m_dropdowns;
	int m_column_count;
	const int m_table_row_count;
	const int m_row_count;

	CSVColumnSettings m_column_settings;
	std::vector<std::vector<float>> m_point_values;
};

}
