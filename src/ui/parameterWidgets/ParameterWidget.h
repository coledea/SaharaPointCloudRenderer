#pragma once

#include "rendering/parameters/AbstractParameter.h"

#include <QGroupBox>
#include <QHBoxLayout>

namespace sahara::ui
{

class ParameterWidget : public QGroupBox
{
	Q_OBJECT

public:
	virtual ~ParameterWidget() = default;

protected:
	ParameterWidget(const rendering::AbstractParameter& parameter, const QString& icon_path, QWidget* parent = nullptr);
	virtual void onParameterValueChanged() = 0;

	QHBoxLayout m_layout;
};

}