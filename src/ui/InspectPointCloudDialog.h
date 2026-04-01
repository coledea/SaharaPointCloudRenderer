#pragma once

#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

#include <QDialog>

namespace Ui
{
class InspectPointCloudDialog;
}

namespace sahara::ui
{

class InspectPointCloudDialog : public QDialog
{
public:
	explicit InspectPointCloudDialog(const rendering::AbstractPointCloudProvider& pointcloud_provider, QWidget* parent = nullptr);
	~InspectPointCloudDialog();

private:
	std::unique_ptr<Ui::InspectPointCloudDialog> m_ui;
	QFont m_bold_font;

	void addNameEntry(const rendering::AbstractPointCloudProvider& pointcloud_provider);
	void addBoundingBoxEntry(const rendering::AbstractPointCloudProvider& pointcloud_provider);
	void addPointNumberEntry(const rendering::AbstractPointCloudProvider& pointcloud_provider);
};

}
