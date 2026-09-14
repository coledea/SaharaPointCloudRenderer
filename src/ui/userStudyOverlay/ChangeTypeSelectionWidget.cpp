#include "ChangeTypeSelectionWidget.h"

#include <QListView>
#include <QVBoxLayout>

namespace sahara::ui
{

ChangeTypeSelectionWidget::ChangeTypeSelectionWidget(QWidget* parent)
	: QFrame(parent)
	, m_panel_label(new QLabel(tr("Study mode"), this))
	, m_mode_combo(new QComboBox(this))
{
	setFocusPolicy(Qt::NoFocus);

	m_mode_combo->setView(new QListView(m_mode_combo));
	m_mode_combo->setFocusPolicy(Qt::NoFocus);

	setFrameShape(QFrame::StyledPanel);
	setStyleSheet("QFrame { background-color: rgba(248, 248, 248, 235); border: 1px solid rgba(35, 35, 35, 48); border-radius: 10px; }"
				  "QLabel { color: rgb(28, 28, 28); font-weight: 600; background: transparent; }"
				  "QComboBox { color: rgb(28, 28, 28); background-color: rgb(255, 255, 255); border: 1px solid rgba(35, 35, 35, 64); border-radius: 6px; padding: 4px 28px 4px 8px; selection-background-color: rgb(217, 232, 255); selection-color: rgb(20, 20, 20); }"
				  "QComboBox QAbstractItemView { color: rgb(20, 20, 20); background-color: rgb(255, 255, 255); border: 1px solid rgba(35, 35, 35, 64); selection-background-color: rgb(217, 232, 255); selection-color: rgb(20, 20, 20); }");

	auto panel_layout = new QVBoxLayout(this);
	panel_layout->setContentsMargins(12, 10, 12, 10);
	panel_layout->setSpacing(6);
	panel_layout->addWidget(m_panel_label);
	panel_layout->addWidget(m_mode_combo);

	m_mode_combo->addItem(tr("Addition"), "addition");
	m_mode_combo->addItem(tr("Removal"), "removal");
	// m_mode_combo->addItem(tr("Transformation"), "transformation");
}

QString ChangeTypeSelectionWidget::currentSelection() const
{
	return m_mode_combo->currentData().toString();
}

}
