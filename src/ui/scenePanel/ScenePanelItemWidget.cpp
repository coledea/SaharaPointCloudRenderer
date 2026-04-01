#include "ScenePanelItemWidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>

namespace sahara::ui
{

sahara::ui::ScenePanelItemWidget::ScenePanelItemWidget(const QString& text)
{
	auto layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	auto visibility_checkbox = new QCheckBox();
	visibility_checkbox->setStyleSheet("QCheckBox:indicator{width:16px; height: 16px;}"
									   "QCheckBox::indicator:unchecked{image: url(:/icons/view-visible.png);}"
									   "QCheckBox::indicator:checked{image: url(:/icons/view-invisible.png);}");
	visibility_checkbox->setFixedSize(QSize(16, 16));

	connect(visibility_checkbox, &QCheckBox::checkStateChanged, [this](Qt::CheckState state) { emit visibilityToggled(!(state == Qt::Checked)); });

	layout->addWidget(visibility_checkbox, 0, Qt::AlignLeft);

	auto label = new QLabel(this);
	label->setText(text);
	label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	label->adjustSize();
	layout->addWidget(label, 1, Qt::AlignLeft);

	this->setLayout(layout);
}

}