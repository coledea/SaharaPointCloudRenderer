#include "ScenePanelItem.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>

namespace sahara::ui
{

ScenePanelItem::ScenePanelItem(uint pointcloud_id) noexcept
	: m_pointcloud_id(pointcloud_id)
{
}

uint ScenePanelItem::pointCloudID() noexcept
{
	return m_pointcloud_id;
}

}