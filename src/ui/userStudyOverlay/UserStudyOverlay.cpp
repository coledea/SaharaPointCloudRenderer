#include "UserStudyOverlay.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace sahara::ui
{

UserStudyOverlay::UserStudyOverlay(QWidget* parent)
	: QWidget(parent)
	, m_has_selected_point(false)
	, m_selected_point(QPoint())
	, m_annotation_panel(new QWidget(this))
	, m_annotation_list(new QListWidget(m_annotation_panel))
	, m_unselect_annotation_button(new QPushButton(tr("Unselect"), m_annotation_panel))
	, m_remove_annotation_button(new QPushButton(tr("Remove"), m_annotation_panel))
{
	setAttribute(Qt::WA_NoSystemBackground);
	setFocusPolicy(Qt::StrongFocus);

	auto layout = new QVBoxLayout(m_annotation_panel);
	layout->setContentsMargins(6, 6, 6, 6);
	layout->setSpacing(4);
	auto button_layout = new QHBoxLayout();
	button_layout->setContentsMargins(0, 0, 0, 0);
	button_layout->setSpacing(4);
	button_layout->addWidget(m_unselect_annotation_button);
	button_layout->addWidget(m_remove_annotation_button);
	layout->addWidget(m_annotation_list);
	layout->addLayout(button_layout);
	m_annotation_panel->setAutoFillBackground(true);
	m_annotation_panel->setStyleSheet("QWidget { background: rgba(20, 20, 20, 210); color: white; } QListWidget { background: rgba(255, 255, 255, 235); color: black; } QPushButton { padding: 3px; }");
	m_unselect_annotation_button->setEnabled(false);
	m_remove_annotation_button->setEnabled(false);

	connect(m_annotation_list, &QListWidget::currentRowChanged, this, [this](int row) {
		m_unselect_annotation_button->setEnabled(row >= 0);
		m_remove_annotation_button->setEnabled(row >= 0);
		emit annotationSelected(row);
	});
	connect(m_unselect_annotation_button, &QPushButton::clicked, this, [this]() {
		m_annotation_list->clearSelection();
		m_annotation_list->setCurrentRow(-1);
		m_unselect_annotation_button->setEnabled(false);
		m_remove_annotation_button->setEnabled(false);
		emit annotationSelected(-1);
	});
	connect(m_remove_annotation_button, &QPushButton::clicked, this, [this]() {
		const int row = m_annotation_list->currentRow();
		if (row >= 0)
		{
			emit annotationRemovalRequested(row);
		}
	});

	m_annotation_panel->hide();
	hide();
}

void UserStudyOverlay::resize(int width, int height)
{
	setGeometry(QRect(0, 0, width, height));
	updateAnnotationPanelGeometry();
}

QPixmap UserStudyOverlay::getFinalPixmap()
{
	QPixmap final_pixmap(width(), height());
	drawImage(&final_pixmap, false);
	return final_pixmap;
}

QPoint UserStudyOverlay::selectedPoint() const noexcept
{
	return m_selected_point;
}

void UserStudyOverlay::open(const QPixmap& pixmap)
{
	m_rendered_image = pixmap;
	m_annotation_panel->show();
	updateAnnotationPanelGeometry();
	show();
	raise();
	setFocus();
	update();
}

void UserStudyOverlay::close()
{
	clearSelectedPoint();
	m_highlighted_annotation_position.reset();
	m_annotation_list->clearSelection();
	m_annotation_list->setCurrentRow(-1);
	m_annotation_panel->hide();
	hide();
}

void UserStudyOverlay::setAnnotationList(const QStringList& annotations)
{
	const QSignalBlocker blocker(m_annotation_list);
	const int current_row = m_annotation_list->currentRow();
	m_annotation_list->clear();
	m_annotation_list->addItems(annotations);
	if (current_row >= 0 && current_row < m_annotation_list->count())
	{
		m_annotation_list->setCurrentRow(current_row);
	}
	m_unselect_annotation_button->setEnabled(m_annotation_list->currentRow() >= 0);
	m_remove_annotation_button->setEnabled(m_annotation_list->currentRow() >= 0);
}

void UserStudyOverlay::setHighlightedAnnotationPosition(std::optional<QPoint> position)
{
	m_highlighted_annotation_position = position;
	update();
}

void UserStudyOverlay::clearSelectedPoint()
{
	m_has_selected_point = false;
	update();
}

void UserStudyOverlay::mousePressEvent(QMouseEvent* event)
{
	if (!isVisible() || event->button() != Qt::LeftButton)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	m_selected_point = event->position().toPoint();
	m_has_selected_point = true;
	update();

	emit pointSelected();
	event->accept();
}

void UserStudyOverlay::keyReleaseEvent(QKeyEvent* event)
{
	if (isVisible() && (event->key() == Qt::Key_Space || event->key() == Qt::Key_Escape))
	{
		emit closeRequested();
		event->accept();
		return;
	}

	QWidget::keyReleaseEvent(event);
}

void UserStudyOverlay::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	drawImage(this, true);
}

void UserStudyOverlay::drawImage(QPaintDevice* device, bool show_banner)
{
	QPainter painter(device);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.fillRect(rect(), QColor(24, 24, 24));

	if (!m_rendered_image.isNull())
	{
		painter.drawPixmap(rect(), m_rendered_image);
	}

	if (show_banner)
	{
		const QRect banner_rect(16, 16, qMax(180, qMin(width() - 32, 360)), 38);
		QPainterPath banner_path;
		banner_path.addRoundedRect(banner_rect, 10.0, 10.0);
		painter.fillPath(banner_path, QColor(20, 20, 20, 185));
		painter.setPen(Qt::white);
		painter.drawText(banner_rect.adjusted(14, 0, -14, 0), Qt::AlignVCenter | Qt::AlignLeft, tr("Paused. Please mark a changed object."));
	}

	if (show_banner && m_highlighted_annotation_position)
	{
		drawAnnotationMarker(painter, *m_highlighted_annotation_position, true);
	}

	if (m_has_selected_point)
	{
		drawAnnotationMarker(painter, m_selected_point, false);
	}
}

void UserStudyOverlay::drawAnnotationMarker(QPainter& painter, const QPoint& position, bool highlighted)
{
	painter.setPen(Qt::NoPen);
	if (highlighted)
	{
		painter.setBrush(QColor(0, 0, 0));
		painter.drawEllipse(position, 17, 17);
		painter.setBrush(QColor(0, 217, 255));
		painter.drawEllipse(position, 12, 12);
	}
	painter.setBrush(QColor(0, 0, 0));
	painter.drawEllipse(position, 12, 12);
	painter.setBrush(QColor(255, 217, 13));
	painter.drawEllipse(position, 7, 7);
}

void UserStudyOverlay::updateAnnotationPanelGeometry()
{
	const int panel_width = qMin(240, qMax(180, width() / 5));
	const int panel_height = qMin(qMax(120, height() / 4), 220);
	m_annotation_panel->setGeometry(width() - panel_width - 8, 8, panel_width, panel_height);
}

}
