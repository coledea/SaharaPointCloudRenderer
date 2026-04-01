#include "ParameterWidgetFactory.h"

#include "BoolParameterWidget.h"
#include "ColorParameterWidget.h"
#include "EnumParameterWidget.h"
#include "FilePathParameterWidget.h"
#include "FloatParameterWidget.h"
#include "IntParameterWidget.h"
#include "TriggerParameterWidget.h"
#include "Vector3DParameterWidget.h"

namespace sahara::ui
{

ParameterWidget* ParameterWidgetFactory::createParameterWidget(rendering::AbstractParameter* parameter)
{
	if (auto int_parameter = dynamic_cast<rendering::RangeParameter<int>*>(parameter))
	{
		return new IntParameterWidget(int_parameter);
	}

	if (auto float_parameter = dynamic_cast<rendering::RangeParameter<float>*>(parameter))
	{
		return new FloatParameterWidget(float_parameter);
	}

	if (auto vector_3d_parameter = dynamic_cast<rendering::RangeParameter<QVector3D>*>(parameter))
	{
		return new Vector3DParameterWidget(vector_3d_parameter);
	}

	if (auto bool_parameter = dynamic_cast<rendering::Parameter<bool>*>(parameter))
	{
		return new BoolParameterWidget(bool_parameter);
	}

	if (auto color_parameter = dynamic_cast<rendering::Parameter<QColor>*>(parameter))
	{
		return new ColorParameterWidget(color_parameter);
	}

	if (auto enum_parameter = dynamic_cast<rendering::EnumParameter*>(parameter))
	{
		return new EnumParameterWidget(enum_parameter);
	}

	if (auto trigger_parameter = dynamic_cast<rendering::TriggerParameter*>(parameter))
	{
		return new TriggerParameterWidget(trigger_parameter);
	}

	if (auto file_path_parameter = dynamic_cast<rendering::FilePathParameter*>(parameter))
	{
		return new FilePathParameterWidget(file_path_parameter);
	}

	assert(false);
}

}