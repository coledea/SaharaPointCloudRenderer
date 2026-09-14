#include "CSVLoadingDialog.h"

#include "CSVColumnTypeDetection.h"
#include "ui_CSVLoadingDialog.h"

#include <QCheckBox>
#include <QFile>
#include <QSpinBox>
#include <cmath>
#include <iostream>

namespace sahara::io
{

const QRegularExpression CSVLoadingDialog::m_header_regex("^//|[a-d]|[f-z]", QRegularExpression::CaseInsensitiveOption);

CSVLoadingDialog::CSVLoadingDialog(QString& file_name, const QString& delimiter, QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::CSVLoadingDialog)
	, m_file_name(file_name)
	, m_table_row_count(10)
	, m_row_count(100)
{
	m_ui->setupUi(this);

	populateTable(delimiter);
}

CSVLoadingDialog::~CSVLoadingDialog()
{
	delete m_ui;
}

const CSVColumnSettings& CSVLoadingDialog::columnSettings()
{
	return m_column_settings;
};

bool CSVLoadingDialog::checkAndSetHeader(QString& first_line, const QString& delimiter)
{
	if (CSVColumnSettings::HeaderExistsSetting::Auto)
	{
		m_column_settings.m_header_exists = m_header_regex.match(first_line).hasMatch() ? CSVColumnSettings::HeaderExistsSetting::Yes : CSVColumnSettings::HeaderExistsSetting::No;
	}

	QString custom_line;
	switch (m_column_settings.m_header_exists)
	{
		case (CSVColumnSettings::HeaderExistsSetting::No):
			m_ui->tableWidget->horizontalHeader()->setVisible(false);

			// still add names for columns though.
			for (int i = 0; i < m_column_count; i++)
			{
				custom_line += "Column" + QString::number(i) + delimiter;
			}
			setHeader(custom_line, delimiter);
			return false;
		case (CSVColumnSettings::HeaderExistsSetting::Yes):
			if (first_line.startsWith("//"))
			{
				first_line.remove("//");
			}
			setHeader(first_line, delimiter);
			return true;
		default:
			return false;
	}
}

void CSVLoadingDialog::setHeader(QString& header_line, const QString& delimiter)
{
	auto header_cells = header_line.split(delimiter);

	for (int column = 0; column < m_column_count; column++)
	{
		auto header_name = header_cells[column].trimmed();

		m_header_names.push_back(header_name);
		m_ui->tableWidget->setHorizontalHeaderItem(column, new QTableWidgetItem(header_name));
	}
}

void CSVLoadingDialog::addDropdowns(int row)
{
	m_dropdowns.resize(m_column_count);

	for (auto& column : m_column_settings.m_columns)
	{
		m_dropdowns[column.index()] = new QComboBox(m_ui->tableWidget);

		for (int columnType = 0; columnType <= static_cast<int>(CSVColumn::ColumnType::None); ++columnType)
		{
			m_dropdowns[column.index()]->addItem(CSVColumn::column_type_strings.at(static_cast<CSVColumn::ColumnType>(columnType)));
		}

		m_dropdowns[column.index()]->setCurrentIndex(static_cast<int>(column.columnType()));
		m_ui->tableWidget->setCellWidget(row, column.index(), m_dropdowns[column.index()]);

		connect(m_dropdowns[column.index()], &QComboBox::currentIndexChanged, [&column, this](int index) {
			this->updateDropdownSelection(column, static_cast<CSVColumn::ColumnType>(index));
		});
	}
	m_ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem("Use as"));
}

void CSVLoadingDialog::updateDropdownSelection(CSVColumn& column, CSVColumn::ColumnType new_type)
{
	CSVColumn::ColumnType old_type = column.columnType();

	if (old_type == new_type)
	{
		return;
	}

	if (m_column_settings.columnAvailable(new_type) && new_type != CSVColumn::ColumnType::None && new_type != CSVColumn::ColumnType::Custom)
	{
		auto other_column = m_column_settings.columnForType(new_type);

		if (column.index() == other_column.index())
		{
			return;
		}

		QComboBox* other_dropdown = m_dropdowns[other_column.index()];
		other_dropdown->setCurrentIndex(static_cast<int>(CSVColumn::ColumnType::None));
	}

	m_column_settings.setColumn(new_type, column);
}

void CSVLoadingDialog::addRow(int row, QString& row_line, const QString& delimiter)
{
	auto cells = row_line.split(delimiter);

	m_ui->tableWidget->setRowCount(m_ui->tableWidget->rowCount() + 1);

	for (int column = 0; column < std::min(static_cast<long long>(m_column_count), cells.size()); column++)
	{
		QString dataString = cells[column];
		m_ui->tableWidget->setItem(row, column, new QTableWidgetItem(dataString));
	}
	m_ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row)));
}

void CSVLoadingDialog::configureTable()
{
	m_ui->tableWidget->clear();
	m_ui->tableWidget->setColumnCount(0);
	m_ui->tableWidget->setRowCount(0);
	m_ui->tableWidget->setColumnCount(m_column_count);
	m_ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ui->tableWidget->setFocusPolicy(Qt::NoFocus);
	m_ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
}

void CSVLoadingDialog::populateTable(const QString& delimiter)
{
	QFile rfile(m_file_name);

	if (!rfile.open(QFile::ReadOnly | QFile::Text))
	{
		std::cout << "Failed to open point cloud file" << std::endl;
		return;
	}

	QTextStream in(&rfile);

	// determine column number
	auto first_line = in.readLine();
	first_line.remove(QRegularExpression("\\s")); // remove whitespace, tabs, etc.
	m_column_count = first_line.count(delimiter) + 1;

	configureTable();
	if (!checkAndSetHeader(first_line, delimiter))
	{
		// go back to keep first line for processing if it is not a header
		in.seek(0);
	}

	for (int col = 0; col < m_column_count; col++)
	{
		m_column_settings.m_columns.emplace_back(CSVColumn(col, m_header_names[col]));
	}

	populateRows(in, delimiter);

	adjustSize();
	activateWindow();
}

void CSVLoadingDialog::populateRows(QTextStream& in, const QString& delimiter)
{
	// add placeholder row for dropdowns
	m_ui->tableWidget->setRowCount(m_ui->tableWidget->rowCount() + 1);
	int dropdown_row_index = m_ui->tableWidget->rowCount() - 1;

	std::vector<bool> float_column(m_column_count, false);

	int row_index_offset = m_ui->tableWidget->rowCount();
	for (int row_index = 0; row_index < m_row_count && !in.atEnd(); row_index++)
	{
		auto line = in.readLine();
		line.remove(QRegularExpression("\\s")); // remove whitespace, tabs, etc.
		if (row_index < m_table_row_count)
		{
			addRow(row_index + row_index_offset, line, delimiter);
		}

		auto strings = line.split(delimiter);
		for (int col = 0; col < strings.size(); ++col)
		{
			auto value = strings[col].toFloat();
			auto& column = m_column_settings.m_columns[col];
			column.updateLimit(value);
			float_column[col] = float_column[col] || strings[col].contains(".");
		}
	}

	for (int col = 0; col < m_column_settings.m_columns.size(); ++col)
	{
		auto& column = m_column_settings.m_columns[col];
		column.setValueType(float_column[col] ? CSVColumn::ValueType::FLOAT : CSVColumn::ValueType::INT);
	}

	CSVColumnTypeDetection::detectColumnTypes(m_column_settings);

	addDropdowns(dropdown_row_index);
}

}
