#include "MultipleParametersWidget.h"

namespace sahara::ui
{

MultipleParametersWidget::MultipleParametersWidget(QWidget* parent)
	: QWidget(parent)
{
	setLayout(&m_layout);
	m_layout.setContentsMargins(2, 2, 2, 2);
	m_layout.setSpacing(10);
	m_layout.addStretch();
}

void MultipleParametersWidget::addParameterWidget(ParameterWidget* parameter_widget)
{
	m_layout.insertWidget(m_layout.count() - 1, parameter_widget); // always insert before the spacer so that the layout remains compact
	this->adjustSize();
}

void MultipleParametersWidget::removeAllParameterWidgets()
{
	while (auto item = m_layout.takeAt(0))
	{
		if (item->widget())
		{
			delete item->widget();
		}
		delete item;
	}
	m_layout.addStretch();
}

}