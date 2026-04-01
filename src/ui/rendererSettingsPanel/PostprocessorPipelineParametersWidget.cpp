#include "PostprocessorPipelineParametersWidget.h"

#include "ui/parameterWidgets/ParameterWidgetFactory.h"

namespace sahara::ui
{

PostprocessorPipelineParametersWidget::PostprocessorPipelineParametersWidget(QWidget* parent)
	: QToolBox(parent)
{
	setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
}

void PostprocessorPipelineParametersWidget::addPostprocessorParameters(const QString& postprocessor_name, const std::vector<rendering::AbstractParameter*>& parameters)
{
	m_parameters.push_back(std::make_unique<MultipleParametersWidget>(this));
	for (auto parameter : parameters)
	{
		m_parameters.back()->addParameterWidget(ParameterWidgetFactory::createParameterWidget(parameter));
	}
	addItem(m_parameters.back().get(), postprocessor_name);
	setItemIcon(count() - 1, QIcon(":/icons/effect.png"));
}

void PostprocessorPipelineParametersWidget::removeCurrentPostprocessorParameters()
{
	int current_index = currentIndex();
	removeItem(current_index);
	m_parameters.erase(m_parameters.begin() + current_index);
}

}