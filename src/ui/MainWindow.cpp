#include "MainWindow.h"

#include "ui_MainWindow.h"
#include "utils/Profiler.h"

#include <QClipboard>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMimeData>

namespace sahara::ui
{

MainWindow::MainWindow(QWidget* parent, RenderWindow* render_window, navigation::NavigationHandler* navigation_handler, navigation::Camera* camera, rendering::Scene* scene)
	: QMainWindow(parent)
	, m_ui(std::make_unique<Ui::MainWindow>())
	, m_render_window(render_window)
	, m_render_window_container(QWidget::createWindowContainer(render_window))
	, m_camera_path_panel(navigation_handler, camera)
	, m_camera_settings_panel(navigation_handler, camera)
	, m_scene_panel(scene)
	, m_renderer_settings_panel(scene)
	, m_scene(scene)
#ifdef ENABLE_USER_STUDY_MODE
	, m_user_study_controller(nullptr)
#endif

{
	m_ui->setupUi(this);
	m_ui->DW_CameraPathPanel->setWidget(&m_camera_path_panel);
	m_ui->DW_CameraSettingsPanel->setWidget(&m_camera_settings_panel);
	m_ui->DW_ScenePanel->setWidget(&m_scene_panel);
	m_ui->DW_RendererSettingsPanel->setWidget(&m_renderer_settings_panel);

	connect(&m_scene_panel, &ScenePanel::activePointCloudChanged, &m_renderer_settings_panel, &RendererSettingsPanel::setActivePointCloud);
	connect(&m_scene_panel, &ScenePanel::pointCloudFocusRequested, m_scene, &rendering::Scene::focusOnPointCloud);

	// we use an additional widget to avoid the render window overlapping the menu bar and docking widgets
	setCentralWidget(new QWidget());
	auto central_layout = new QHBoxLayout(centralWidget());
	central_layout->setContentsMargins(10, 30, 10, 10);
	central_layout->addWidget(m_render_window_container, Qt::AlignCenter);

	connect(m_render_window, &RenderWindow::receivedDrop, this, &MainWindow::dropEvent, Qt::ConnectionType::DirectConnection);
	connect(m_render_window, &RenderWindow::resized, [camera, this]() { camera->setViewport(m_render_window->deviceScaledWidth(), m_render_window->deviceScaledHeight()); });

#ifdef ENABLE_USER_STUDY_MODE
	m_user_study_controller = std::make_unique<UserStudyController>(centralWidget(), m_render_window, &m_scene_panel, camera, m_scene);
#endif

	connect(m_ui->DW_CameraPathPanel, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		const QSignalBlocker blocker(m_ui->actionToggleCameraPathPanel);
		m_ui->actionToggleCameraPathPanel->setChecked(visible); });

	connect(m_ui->DW_CameraSettingsPanel, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		const QSignalBlocker blocker(m_ui->actionToggleCameraSettingsPanel);
		m_ui->actionToggleCameraSettingsPanel->setChecked(visible); });

	connect(m_ui->DW_ScenePanel, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		const QSignalBlocker blocker(m_ui->actionToggleScenePanel);
		m_ui->actionToggleScenePanel->setChecked(visible); });

	connect(m_ui->DW_RendererSettingsPanel, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		const QSignalBlocker blocker(m_ui->actionToggleRendererSettingsPanel);
		m_ui->actionToggleRendererSettingsPanel->setChecked(visible); });
}

MainWindow::~MainWindow()
{
	m_render_window->setParent(nullptr); // release ownership
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (m_render_window != nullptr)
	{
		m_render_window->shutdownRendering();
	}

	QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		// TODO: could check for specific file extensions here
		event->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent* event)
{
	for (const auto& url : event->mimeData()->urls())
	{
		m_scene->addPointCloud(url.toLocalFile());
	}
	event->setDropAction(Qt::DropAction::IgnoreAction);
	event->acceptProposedAction();
}

void MainWindow::on_actionReloadShader_triggered()
{
	m_scene->reloadShaders();
}

void MainWindow::on_actionCopyFramebufferToClipboard_triggered()
{
	QClipboard* clipboard = QGuiApplication::clipboard();
	clipboard->setPixmap(screen()->grabWindow(m_render_window->winId()));
}

void MainWindow::on_actionSaveFramebufferToFile_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Framebuffer"), "./", tr("Image Files (*.png *.jpg *.bmp)"));
	if (!fileName.isEmpty())
	{
		screen()->grabWindow(m_render_window->winId()).save(fileName);
	}
}

void MainWindow::on_actionSaveFramebufferToFiles_toggled(bool checked)
{
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::on_actionOpenFile_triggered()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("Point Cloud Files (*.ply *.csv *.txt *.ascii *.xyz *.mtpc *.json)"));
	m_scene->addPointCloud(filepath);
}

void MainWindow::on_actionOpenSettings_triggered()
{
}

void MainWindow::on_actionToggleCameraPathPanel_toggled()
{
	const QSignalBlocker blocker(m_ui->DW_CameraPathPanel);
	m_ui->DW_CameraPathPanel->isHidden() ? m_ui->DW_CameraPathPanel->show() : m_ui->DW_CameraPathPanel->hide();
}

void MainWindow::on_actionToggleCameraSettingsPanel_toggled()
{
	const QSignalBlocker blocker(m_ui->DW_CameraSettingsPanel);
	m_ui->DW_CameraSettingsPanel->isHidden() ? m_ui->DW_CameraSettingsPanel->show() : m_ui->DW_CameraSettingsPanel->hide();
}

void MainWindow::on_actionToggleRendererSettingsPanel_toggled()
{
	const QSignalBlocker blocker(m_ui->DW_RendererSettingsPanel);
	m_ui->DW_RendererSettingsPanel->isHidden() ? m_ui->DW_RendererSettingsPanel->show() : m_ui->DW_RendererSettingsPanel->hide();
}

void MainWindow::on_actionToggleScenePanel_toggled()
{
	const QSignalBlocker blocker(m_ui->DW_ScenePanel);
	m_ui->DW_ScenePanel->isHidden() ? m_ui->DW_ScenePanel->show() : m_ui->DW_ScenePanel->hide();
}

void MainWindow::on_actionToggleProfiling_triggered()
{

	if (m_ui->actionToggleProfiling->text() == "Start Profiling")
	{
		if (!utils::global_profiler.isRunning())
		{
			utils::global_profiler.startSession("./profiler_results", 10000);
			m_ui->actionToggleProfiling->setText("Stop Profiling");
		}
	}
	else
	{
		utils::global_profiler.endSession();
		m_ui->actionToggleProfiling->setText("Start Profiling");
	}
}

}
