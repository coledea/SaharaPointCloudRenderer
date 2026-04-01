#pragma once

#include "ParametersPanel.h"
#include "PostprocessorPipelineParametersPanel.h"
#include "rendering/Scene.h"

#include <QWidget>

namespace Ui
{
class RendererSettingsPanel;
}

namespace sahara::ui
{

class RendererSettingsPanel : public QWidget
{
	Q_OBJECT

public:
	RendererSettingsPanel(rendering::Scene* scene, QWidget* parent = nullptr);
	~RendererSettingsPanel();

	void setActivePointCloud(uint id);

private:
	std::unique_ptr<Ui::RendererSettingsPanel> m_ui;
	ParametersPanel m_pointcloud_provider_parameters;
	ParametersPanel m_rasterizer_parameters;
	ParametersPanel m_colorizer_parameters;
	PostprocessorPipelineParametersPanel m_postprocessor_parameters;
	rendering::Scene* m_scene;
	uint m_active_pointcloud_id;
};

}