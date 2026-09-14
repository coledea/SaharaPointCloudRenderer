#include "ScenePanelItem.h"

namespace sahara::ui
{

ScenePanelItem::ScenePanelItem(uint pointcloud_id, const QString& pointcloud_name) noexcept
	: m_pointcloud_id(pointcloud_id)
	, m_pointcloud_name(pointcloud_name)
{
}

uint ScenePanelItem::pointCloudID() noexcept
{
	return m_pointcloud_id;
}

const QString& ScenePanelItem::pointCloudName() const noexcept
{
	return m_pointcloud_name;
}

}
