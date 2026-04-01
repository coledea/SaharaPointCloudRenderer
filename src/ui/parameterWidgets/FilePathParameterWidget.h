#pragma once

#include "FileSelector.h"
#include "ParameterWidget.h"
#include "rendering/parameters/FilePathParameter.h"

namespace sahara::ui
{

class FilePathParameterWidget : public ParameterWidget
{
public:
	FilePathParameterWidget(rendering::FilePathParameter* parameter, QWidget* parent = nullptr);

private:
	rendering::FilePathParameter* m_parameter;
	FileSelector m_file_selector;

	void onParameterValueChanged() override;
};

}