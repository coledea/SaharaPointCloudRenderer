#pragma once

#include "RenderWindow.h"
#include "cameraPanels/CameraPathPanel.h"
#include "cameraPanels/CameraSettingsPanel.h"
#include "navigation/NavigationHandler.h"
#include "rendererSettingsPanel/RendererSettingsPanel.h"
#include "scenePanel/ScenePanel.h"

#ifdef ENABLE_USER_STUDY_MODE
#	include "userStudyOverlay/UserStudyController.h"
#endif

#include <QCloseEvent>
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

namespace sahara::ui
{

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent, RenderWindow* render_window, navigation::NavigationHandler* navigation_handler, navigation::Camera* camera, rendering::Scene* scene);
	~MainWindow();

	void closeEvent(QCloseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

public slots:

	void on_actionReloadShader_triggered();
	void on_actionCopyFramebufferToClipboard_triggered();
	void on_actionSaveFramebufferToFile_triggered();
	void on_actionSaveFramebufferToFiles_toggled(bool checked);

	void on_actionExit_triggered();
	void on_actionOpenFile_triggered();
	void on_actionOpenSettings_triggered();
	void on_actionToggleCameraPathPanel_toggled();
	void on_actionToggleCameraSettingsPanel_toggled();
	void on_actionToggleRendererSettingsPanel_toggled();
	void on_actionToggleScenePanel_toggled();

	void on_actionToggleProfiling_triggered();

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	RenderWindow* m_render_window;
	CameraPathPanel m_camera_path_panel;
	CameraSettingsPanel m_camera_settings_panel;
	ScenePanel m_scene_panel;
	RendererSettingsPanel m_renderer_settings_panel;

	rendering::Scene* m_scene;
	QWidget* m_render_window_container;
#ifdef ENABLE_USER_STUDY_MODE
	std::unique_ptr<UserStudyController> m_user_study_controller;
#endif
};

}
