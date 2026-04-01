#include "FilePathParameterWidget.h"

namespace sahara::ui
{

FilePathParameterWidget::FilePathParameterWidget(rendering::FilePathParameter* parameter, QWidget* parent)
	: ParameterWidget(
		  *parameter,
		  ":/icons/parameter-custom.png",
		  parent)
	, m_parameter(parameter)
	, m_file_selector(parameter->value(), parameter->filetypes(), this)
{

	m_layout.insertWidget(1, &m_file_selector); // insert after icon
	connect(&m_file_selector, &FileSelector::pathChanged, m_parameter, &rendering::FilePathParameter::setValue);
}

void FilePathParameterWidget::onParameterValueChanged()
{
	auto new_path = m_parameter->value();
	if (new_path != m_file_selector.path())
	{
		const QSignalBlocker blocker(&m_file_selector);
		m_file_selector.setPath(new_path);
	}
}
}