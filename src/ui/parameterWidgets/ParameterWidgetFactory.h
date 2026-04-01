#pragma once

#include "ParameterWidget.h"

namespace sahara::ui
{

class ParameterWidgetFactory
{
public:
	static ParameterWidget* createParameterWidget(rendering::AbstractParameter* parameter);
};

}