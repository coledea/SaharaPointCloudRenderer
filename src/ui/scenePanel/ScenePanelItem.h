#pragma once

#include <QListWidgetItem>

namespace sahara::ui
{

class ScenePanelItem : public QListWidgetItem
{
public:
	ScenePanelItem(uint pointcloud_id, const QString& pointcloud_name) noexcept;
	uint pointCloudID() noexcept;
	const QString& pointCloudName() const noexcept;

private:
	uint m_pointcloud_id;
	QString m_pointcloud_name;
};

}
