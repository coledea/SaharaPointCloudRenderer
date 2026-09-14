#pragma once

#include "ui/parameterWidgets/MultipleParametersWidget.h"

#include <QToolBox>

namespace sahara::ui
{

class PostprocessorPipelineParametersWidget : public QToolBox
{
public:
	PostprocessorPipelineParametersWidget(QWidget* parent = nullptr);

	void addPostprocessorParameters(const QString& postprocessor_name, const std::vector<rendering::AbstractParameter*>& parameters);
	void removePostprocessorParameters(int index);

private:
	std::vector<std::unique_ptr<MultipleParametersWidget>> m_parameters;
};

}