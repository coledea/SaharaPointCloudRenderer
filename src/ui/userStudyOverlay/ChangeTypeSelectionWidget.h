#pragma once

#include <QComboBox>
#include <QFrame>
#include <QLabel>

namespace sahara::ui
{

class ChangeTypeSelectionWidget : public QFrame
{
	Q_OBJECT

public:
	explicit ChangeTypeSelectionWidget(QWidget* parent = nullptr);
	QString currentSelection() const;

private:
	QLabel* m_panel_label;
	QComboBox* m_mode_combo;
};

}
