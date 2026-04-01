#pragma once

#include "ParameterWidget.h"

#include <QGridLayout>

namespace sahara::ui
{

class MultipleParametersWidget : public QWidget
{
public:
	MultipleParametersWidget(QWidget* parent = nullptr);

	void addParameterWidget(ParameterWidget* parameter_widget);
	void removeAllParameterWidgets();

private:
	QVBoxLayout m_layout;
};

}