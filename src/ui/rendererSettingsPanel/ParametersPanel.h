#pragma once

#include "rendering/Scene.h"
#include "ui/parameterWidgets/MultipleParametersWidget.h"
#include "ui/parameterWidgets/ParameterWidget.h"

#include <QWidget>

namespace Ui
{
class ParametersPanel;
}

namespace sahara::ui
{

class ParametersPanel : public QWidget
{
	Q_OBJECT

public:
	ParametersPanel(rendering::RendererModule renderer_module, rendering::Scene* scene, bool allow_module_selection, QWidget* parent = nullptr);
	~ParametersPanel();

	void setActivePointCloud(uint id);

private:
	std::unique_ptr<Ui::ParametersPanel> m_ui;
	const rendering::RendererModule m_renderer_module;
	rendering::Scene* m_scene;
	uint m_active_pointcloud_id;

	bool m_allow_module_selection;
	std::unordered_map<uint, std::unique_ptr<MultipleParametersWidget>> m_parameters;

	void onPointcloudRemoved(uint id);
	void onPointcloudAdded(uint id);
	void recreateParameterWidgets(uint id);
	void updateModuleChoicesForCurrentPointcloud();
};

}