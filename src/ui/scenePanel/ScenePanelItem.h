#pragma once

#include <QListWidgetItem>

namespace sahara::ui
{

class ScenePanelItem : public QListWidgetItem
{
public:
	ScenePanelItem(uint pointcloud_id) noexcept;
	uint pointCloudID() noexcept;

private:
	uint m_pointcloud_id;
};

}