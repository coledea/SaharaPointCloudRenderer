#include "InspectPointCloudDialog.h"

#include "ui_InspectPointCloudDialog.h"
#include "utils/QVector3DUtilities.h"

#include <QLayout>
#include <QTableWidgetItem>
#include <sstream>

namespace sahara::ui
{

InspectPointCloudDialog::InspectPointCloudDialog(const rendering::AbstractPointCloudProvider& pointcloud_provider, QWidget* parent)
	: QDialog(parent)
	, m_ui(std::make_unique<Ui::InspectPointCloudDialog>())
{
	m_ui->setupUi(this);
	this->layout()->setSizeConstraint(QLayout::SetFixedSize);

	m_ui->propertyTable->setColumnCount(2);
	m_ui->propertyTable->setRowCount(3);

	m_bold_font.setBold(true);

	addNameEntry(pointcloud_provider);
	addPointNumberEntry(pointcloud_provider);
	addBoundingBoxEntry(pointcloud_provider);

	m_ui->propertyTable->resizeColumnsToContents();
	m_ui->propertyTable->resizeRowsToContents();
	m_ui->propertyTable->adjustSize();
	this->adjustSize();
}

InspectPointCloudDialog::~InspectPointCloudDialog()
{
	// We need a destructor here, as Ui::InspectPointCloudDialog was forward-declared
}

void InspectPointCloudDialog::addNameEntry(const rendering::AbstractPointCloudProvider& pointcloud_provider)
{
	QTableWidgetItem* name_key = new QTableWidgetItem("Name");
	name_key->setFont(m_bold_font);
	m_ui->propertyTable->setItem(0, 0, name_key);

	QTableWidgetItem* name_value = new QTableWidgetItem(pointcloud_provider.name());
	name_value->setTextAlignment(Qt::AlignCenter);
	m_ui->propertyTable->setItem(0, 1, name_value);
}

void InspectPointCloudDialog::addPointNumberEntry(const rendering::AbstractPointCloudProvider& pointcloud_provider)
{
	QTableWidgetItem* point_count_key = new QTableWidgetItem("# Points");
	point_count_key->setFont(m_bold_font);
	m_ui->propertyTable->setItem(1, 0, point_count_key);

	QTableWidgetItem* point_count_value = new QTableWidgetItem(QString::number(pointcloud_provider.numberOfPoints()));
	point_count_value->setTextAlignment(Qt::AlignCenter);
	m_ui->propertyTable->setItem(1, 1, point_count_value);
}

void InspectPointCloudDialog::addBoundingBoxEntry(const rendering::AbstractPointCloudProvider& pointcloud_provider)
{
	QTableWidgetItem* bbox_key = new QTableWidgetItem("Bounding Box");
	bbox_key->setFont(m_bold_font);
	m_ui->propertyTable->setItem(2, 0, bbox_key);

	QString bbox_string;
	bbox_string += vectorToString(pointcloud_provider.boundingBox().minimum()) + "\n";
	bbox_string += vectorToString(pointcloud_provider.boundingBox().maximum());

	QTableWidgetItem* bbox_value = new QTableWidgetItem(bbox_string);
	bbox_value->setTextAlignment(Qt::AlignCenter);
	m_ui->propertyTable->setItem(2, 1, bbox_value);
}

}
