#pragma once

#include "ChangeTypeSelectionWidget.h"

#include <QListWidget>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QWidget>
#include <optional>

namespace sahara::ui
{

class UserStudyOverlay : public QWidget
{
	Q_OBJECT

public:
	explicit UserStudyOverlay(QWidget* parent = nullptr);

	QPixmap getFinalPixmap();
	QPoint selectedPoint() const noexcept;

	void open(const QPixmap& pixmap);
	void close();
	void setAnnotationList(const QStringList& annotations);
	void setHighlightedAnnotationPosition(std::optional<QPoint> position);

	void clearSelectedPoint();
	void resize(int width, int height);

signals:
	void pointSelected();
	void closeRequested();
	void annotationSelected(int index);
	void annotationRemovalRequested(int index);

protected:
	void keyReleaseEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

private:
	QPixmap m_rendered_image;
	bool m_has_selected_point;
	QPoint m_selected_point;
	QWidget* m_annotation_panel;
	QListWidget* m_annotation_list;
	QPushButton* m_unselect_annotation_button;
	QPushButton* m_remove_annotation_button;
	std::optional<QPoint> m_highlighted_annotation_position;

	void drawImage(QPaintDevice* device, bool show_banner);
	void drawAnnotationMarker(QPainter& painter, const QPoint& position, bool highlighted);
	void updateAnnotationPanelGeometry();
};

}
