#pragma once

#include "UserStudyOverlay.h"
#include "navigation/Camera.h"
#include "rendering/Scene.h"
#include "ui/RenderWindow.h"
#include "ui/scenePanel/ScenePanel.h"

#include <QObject>
#include <optional>
#include <vector>

namespace sahara::ui
{

class UserStudyController : public QObject
{
	Q_OBJECT

public:
	UserStudyController(QWidget* parent, RenderWindow* render_window, ScenePanel* scene_panel, navigation::Camera* camera, rendering::Scene* scene);

private:
	QWidget* m_dialog_parent;
	RenderWindow* m_render_window;
	ScenePanel* m_scene_panel;
	navigation::Camera* m_camera;
	rendering::Scene* m_scene;
	UserStudyOverlay* m_overlay;
	std::vector<float> m_depth_buffer_snapshot;
	int m_depth_buffer_width = 0;
	int m_depth_buffer_height = 0;

	void updateVisibility(bool open);
	void openOverlay();
	void closeOverlay();
	void resizeOverlay();

	void onPointSelected();
	void onAnnotationSelected(int index);
	void onAnnotationRemovalRequested(int index);
	void refreshAnnotationList();
	std::optional<QPoint> projectAnnotationToOverlay(uint pointcloud_id, const QVector3D& position) const;
	std::optional<QVector3D> selectedWorldPosition(uint pointcloud_id) const;
	QPixmap captureRenderWindow() const;
	QString captureOutputDirectoryPath() const;
	QString currentSceneFilenameStem() const;
	bool saveAnnotations() const;
	QString saveCapture(const QString& change_type) const;
	QString sanitizeFilenameComponent(const QString& text) const;
};

}
