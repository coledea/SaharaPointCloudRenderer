#include "ParameterWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace sahara::ui
{

ParameterWidget::ParameterWidget(const rendering::AbstractParameter& parameter, const QString& icon_path, QWidget* parent)
	: QGroupBox(parameter.name(), parent)
{
	m_layout.setContentsMargins(0, 0, 0, 4);
	m_layout.setSpacing(0);

	auto tool_button_reset = new QToolButton();
	tool_button_reset->setIcon(QIcon(":/icons/general-reset.png"));
	tool_button_reset->setToolTip(tr("Revert parameter to default value"));
	tool_button_reset->setAutoRaise(true);
	tool_button_reset->setAutoFillBackground(false);
	tool_button_reset->setStyleSheet("QToolButton {background-color: transparent;}");

	auto tool_button_resetWidget = new QWidget();
	auto layout_reset_button = new QHBoxLayout(tool_button_resetWidget);
	layout_reset_button->addWidget(tool_button_reset);
	layout_reset_button->setAlignment(Qt::AlignCenter);
	layout_reset_button->setContentsMargins(0, 0, 0, 0);

	auto icon = new QLabel();
	icon->setPixmap(QPixmap(icon_path).scaled(25, 25));
	icon->setAlignment(Qt::AlignVCenter);

	m_layout.addWidget(icon);
	m_layout.addWidget(tool_button_resetWidget);
	setLayout(&m_layout);
	setVisible(parameter.isVisible());

	connect(tool_button_reset, &QToolButton::clicked, &parameter, &rendering::AbstractParameter::reset);
	connect(&parameter, &rendering::AbstractParameter::valueChanged, this, &ParameterWidget::onParameterValueChanged);
	connect(&parameter, &rendering::AbstractParameter::visibilityChanged, this, &ParameterWidget::setVisible);
}

}
