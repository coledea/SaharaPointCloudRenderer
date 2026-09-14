#include "UserStudyController.h"

#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QTextStream>
#include <QTimer>
#include <QVector4D>
#include <cmath>

namespace sahara::ui
{

UserStudyController::UserStudyController(QWidget* parent, RenderWindow* render_window, ScenePanel* scene_panel, navigation::Camera* camera, rendering::Scene* scene)
	: QObject(parent)
	, m_render_window(render_window)
	, m_scene_panel(scene_panel)
	, m_camera(camera)
	, m_scene(scene)
	, m_dialog_parent(parent)
	, m_overlay(new UserStudyOverlay(parent))
{
	resizeOverlay();

	connect(m_render_window, &RenderWindow::pauseStateChanged, this, &UserStudyController::updateVisibility);
	connect(m_overlay, &UserStudyOverlay::pointSelected, this, &UserStudyController::onPointSelected);
	connect(m_overlay, &UserStudyOverlay::closeRequested, m_render_window, &RenderWindow::togglePause);
	connect(m_overlay, &UserStudyOverlay::annotationSelected, this, &UserStudyController::onAnnotationSelected);
	connect(m_overlay, &UserStudyOverlay::annotationRemovalRequested, this, &UserStudyController::onAnnotationRemovalRequested);
	connect(m_scene, &rendering::Scene::userStudyAnnotationsChanged, this, &UserStudyController::refreshAnnotationList);
}

void UserStudyController::updateVisibility(bool open)
{
	open ? openOverlay() : closeOverlay();
}

void UserStudyController::openOverlay()
{
	resizeOverlay();
	m_scene->setUserStudyAnnotationsVisible(false);
	m_render_window->renderOnce();

	const auto depth_buffer = m_render_window->depthBuffer();
	m_depth_buffer_snapshot = depth_buffer.value_or(std::vector<float>());
	m_depth_buffer_width = depth_buffer ? m_render_window->deviceScaledWidth() : 0;
	m_depth_buffer_height = depth_buffer ? m_render_window->deviceScaledHeight() : 0;
	refreshAnnotationList();
	m_overlay->open(captureRenderWindow());
	m_render_window->hide();
}

void UserStudyController::closeOverlay()
{
	m_overlay->close();
	m_depth_buffer_snapshot.clear();
	m_depth_buffer_width = 0;
	m_depth_buffer_height = 0;
	m_scene->setHighlightedUserStudyAnnotation(std::nullopt);
	m_scene->setUserStudyAnnotationsVisible(true);
	m_render_window->show();
	QTimer::singleShot(0, m_render_window, &QWindow::requestActivate);
	QTimer::singleShot(0, m_render_window, &QWindow::requestUpdate);
}

void UserStudyController::resizeOverlay()
{
	m_overlay->resize(m_render_window->frameGeometry().width(), m_render_window->frameGeometry().height());
}

void UserStudyController::onPointSelected()
{
	QMessageBox message_box(m_dialog_parent);
	message_box.setWindowTitle("Confirm marked location");
	message_box.setText("What kind of change did you mark?");
	auto addition_button = message_box.addButton("Addition", QMessageBox::YesRole);
	auto removal_button = message_box.addButton("Removal", QMessageBox::NoRole);
	auto cancel_button = message_box.addButton("Cancel", QMessageBox::RejectRole);
	message_box.setDefaultButton(cancel_button);
	message_box.exec();

	const auto clicked_button = message_box.clickedButton();
	if (clicked_button == cancel_button || (clicked_button != addition_button && clicked_button != removal_button))
	{
		m_overlay->clearSelectedPoint();
		return;
	}

	const auto pointcloud_id = m_scene_panel->activePointCloudID();
	if (!pointcloud_id)
	{
		QMessageBox::warning(m_dialog_parent, tr("No point cloud selected"), tr("No active point cloud is selected."));
		m_overlay->clearSelectedPoint();
		return;
	}

	const auto world_position = selectedWorldPosition(*pointcloud_id);
	if (!world_position)
	{
		QMessageBox::warning(m_dialog_parent, tr("No point selected"), tr("The selected pixel does not contain a rendered point."));
		m_overlay->clearSelectedPoint();
		return;
	}

	const QString change_type = clicked_button == addition_button ? "Addition" : "Removal";
	const QString saved_file = saveCapture(change_type);
	if (saved_file.isEmpty())
	{
		QMessageBox::warning(m_dialog_parent, tr("Save failed"), tr("The image could not be written to disk."));
		m_overlay->clearSelectedPoint();
		return;
	}

	m_scene->addUserStudyAnnotation(*pointcloud_id, change_type, *world_position);
	if (!saveAnnotations())
	{
		QMessageBox::warning(m_dialog_parent, tr("Save failed"), tr("The annotations file could not be written to disk."));
	}
	m_render_window->togglePause();
}

void UserStudyController::onAnnotationSelected(int index)
{
	if (index < 0)
	{
		m_scene->setHighlightedUserStudyAnnotation(std::nullopt);
		m_overlay->setHighlightedAnnotationPosition(std::nullopt);
		return;
	}

	const auto& annotations = m_scene->userStudyAnnotations();
	const auto annotation_index = static_cast<size_t>(index);
	if (annotation_index >= annotations.size())
	{
		return;
	}

	m_scene->setHighlightedUserStudyAnnotation(annotation_index);
	m_overlay->setHighlightedAnnotationPosition(projectAnnotationToOverlay(annotations[annotation_index].pointcloud_id, annotations[annotation_index].position));
}

void UserStudyController::onAnnotationRemovalRequested(int index)
{
	if (index < 0)
	{
		return;
	}

	m_scene->removeUserStudyAnnotation(static_cast<size_t>(index));
	m_overlay->setHighlightedAnnotationPosition(std::nullopt);
	if (!saveAnnotations())
	{
		QMessageBox::warning(m_dialog_parent, tr("Save failed"), tr("The annotations file could not be written to disk."));
	}
}

void UserStudyController::refreshAnnotationList()
{
	QStringList entries;
	const auto& annotations = m_scene->userStudyAnnotations();
	for (size_t i = 0; i < annotations.size(); ++i)
	{
		const auto& annotation = annotations[i];
		entries.push_back(QString("%1. %2 (%3, %4, %5)")
							  .arg(i + 1)
							  .arg(annotation.type)
							  .arg(annotation.position.x(), 0, 'f', 3)
							  .arg(annotation.position.y(), 0, 'f', 3)
							  .arg(annotation.position.z(), 0, 'f', 3));
	}
	m_overlay->setAnnotationList(entries);
}

std::optional<QPoint> UserStudyController::projectAnnotationToOverlay(uint pointcloud_id, const QVector3D& position) const
{
	const qreal device_pixel_ratio = m_render_window->devicePixelRatio();
	for (const auto& annotation_viewport : m_scene->annotationViewports(pointcloud_id))
	{
		const QVector4D clip = annotation_viewport.view_projection_matrix * QVector4D(position, 1.0f);
		if (clip.w() <= 0.0f)
		{
			continue;
		}

		const QVector3D ndc = (clip / clip.w()).toVector3D();
		if (ndc.x() < -1.0f || ndc.x() > 1.0f || ndc.y() < -1.0f || ndc.y() > 1.0f || ndc.z() < -1.0f || ndc.z() > 1.0f)
		{
			continue;
		}

		const int x = static_cast<int>((annotation_viewport.viewport.x() + (ndc.x() + 1.0f) * 0.5f * static_cast<float>(annotation_viewport.viewport.width())) / device_pixel_ratio);
		const int y = static_cast<int>((annotation_viewport.viewport.y() + (1.0f - ndc.y()) * 0.5f * static_cast<float>(annotation_viewport.viewport.height())) / device_pixel_ratio);
		return QPoint(x, y);
	}
	return std::nullopt;
}

std::optional<QVector3D> UserStudyController::selectedWorldPosition(uint pointcloud_id) const
{
	if (m_depth_buffer_snapshot.empty() || m_depth_buffer_width <= 0 || m_depth_buffer_height <= 0)
	{
		return std::nullopt;
	}

	const QPoint selected_point = m_overlay->selectedPoint();
	const qreal device_pixel_ratio = m_render_window->devicePixelRatio();
	const int x = static_cast<int>(std::ceil(static_cast<qreal>(selected_point.x()) * device_pixel_ratio));
	const int y = static_cast<int>(std::ceil(static_cast<qreal>(selected_point.y()) * device_pixel_ratio));

	if (x < 0 || y < 0 || x >= m_depth_buffer_width || y >= m_depth_buffer_height)
	{
		return std::nullopt;
	}

	const auto annotation_viewport = m_scene->annotationViewportAt(pointcloud_id, QPoint(x, y));
	if (!annotation_viewport)
	{
		return std::nullopt;
	}

	const int gl_y = m_depth_buffer_height - y - 1;
	const float depth = m_depth_buffer_snapshot[static_cast<size_t>(gl_y) * static_cast<size_t>(m_depth_buffer_width) + static_cast<size_t>(x)];
	if (depth >= 1.0f)
	{
		return std::nullopt;
	}

	const float width = static_cast<float>(annotation_viewport->viewport.width());
	const float height = static_cast<float>(annotation_viewport->viewport.height());
	const float viewport_x = static_cast<float>(x - annotation_viewport->viewport.x());
	const float viewport_y = static_cast<float>(y - annotation_viewport->viewport.y());

	QVector4D ndc(2.0f * (viewport_x + 0.5f) / width - 1.0f, 1.0f - 2.0f * (viewport_y + 0.5f) / height, depth * 2.0f - 1.0f, 1.0f);

	bool invertible = false;
	const QMatrix4x4 inverse_view_projection = annotation_viewport->view_projection_matrix.inverted(&invertible);
	if (!invertible)
	{
		return std::nullopt;
	}

	QVector4D world = inverse_view_projection * ndc;
	if (qFuzzyIsNull(world.w()))
	{
		return std::nullopt;
	}
	world /= world.w();
	return world.toVector3D();
}

QPixmap UserStudyController::captureRenderWindow() const
{
	return m_dialog_parent->screen()->grabWindow(m_render_window->winId());
}

QString UserStudyController::captureOutputDirectoryPath() const
{
	const QString output_root = "./";
	QDir output_dir(output_root.isEmpty() ? QDir::currentPath() : output_root);
	if (!output_dir.exists("user_study_captures") && !output_dir.mkpath("user_study_captures"))
	{
		return QString();
	}
	if (!output_dir.cd("user_study_captures"))
	{
		return QString();
	}
	return output_dir.absolutePath();
}

bool UserStudyController::saveAnnotations() const
{
	const QString output_dir_path = captureOutputDirectoryPath();
	if (output_dir_path.isEmpty())
	{
		return false;
	}

	QDir output_dir(output_dir_path);
	QFile file(output_dir.filePath(QString("%1.txt").arg(currentSceneFilenameStem())));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		return false;
	}

	QTextStream stream(&file);
	for (const auto& annotation : m_scene->userStudyAnnotations())
	{
		stream << annotation.type << " " << annotation.position.x() << " " << annotation.position.y() << " " << annotation.position.z() << "\n";
	}
	return true;
}

QString UserStudyController::currentSceneFilenameStem() const
{
	const QString pointcloud_name = sanitizeFilenameComponent(m_scene_panel->activePointCloudName());
	return pointcloud_name.isEmpty() ? QString("annotations") : pointcloud_name;
}

QString UserStudyController::saveCapture(const QString& change_type) const
{
	QPixmap pixmap = m_overlay->getFinalPixmap();
	if (pixmap.isNull())
	{
		return QString();
	}

	const QString output_dir_path = captureOutputDirectoryPath();
	if (output_dir_path.isEmpty())
	{
		return QString();
	}
	QDir output_dir(output_dir_path);

	const QString timestamp = QDateTime::currentDateTime().toString("HHmmss_zzz");
	const QString filepath = output_dir.filePath(QString("%1_%2_%3.png").arg(currentSceneFilenameStem(), timestamp, change_type));

	return pixmap.save(filepath) ? filepath : QString();
}

QString UserStudyController::sanitizeFilenameComponent(const QString& text) const
{
	QString sanitized = text;
	sanitized.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
	sanitized = sanitized.trimmed();
	return sanitized.isEmpty() ? QString("capture") : sanitized;
}

}
