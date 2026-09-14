#pragma once

#include "ScenePanelItem.h"
#include "ScenePanelItemWidget.h"
#include "rendering/Scene.h"

#include <QWidget>
#include <optional>

namespace Ui
{
class ScenePanel;
}

namespace sahara::ui
{

class ScenePanel : public QWidget
{
	Q_OBJECT

public:
	explicit ScenePanel(rendering::Scene* scene, QWidget* parent = nullptr);
	~ScenePanel();
	std::optional<uint> activePointCloudID() const;
	QString activePointCloudName() const;

public slots:

	void onPointCloudAdded(const rendering::AbstractPointCloudProvider& pointcloud_provider);

signals:

	void activePointCloudChanged(uint pointcloud_id);
	void pointCloudFocusRequested(uint pointcloud_id);

private:
	std::unique_ptr<Ui::ScenePanel> m_ui;
	rendering::Scene* m_scene;

	void openElementContextMenu(const QPoint& position);
	void focusPointCloud(const QModelIndex& index);
	void removePointCloud(const QModelIndex& index);
	void showPointCloudDetails(const QModelIndex& index);
	void onItemSelectionChanged();
};

}
