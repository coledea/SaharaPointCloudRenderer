#pragma once

#include "PostprocessorPipelineParametersWidget.h"
#include "rendering/Scene.h"

#include <QMenu>

namespace Ui
{
class PostprocessorPipelineParametersPanel;
}

namespace sahara::ui
{

class PostprocessorPipelineParametersPanel : public QWidget
{
	Q_OBJECT

public:
	PostprocessorPipelineParametersPanel(rendering::Scene* scene, QWidget* parent = nullptr);
	~PostprocessorPipelineParametersPanel();

	void setActivePointCloud(uint id);

private:
	std::unique_ptr<Ui::PostprocessorPipelineParametersPanel> m_ui;
	rendering::Scene* m_scene;
	uint m_active_pointcloud_id;
	QMenu m_add_postprocessor_menu;

	std::unordered_map<uint, std::unique_ptr<PostprocessorPipelineParametersWidget>> m_parameters;

private slots:
	void onPointcloudRemoved(uint id);
	void onPointcloudAdded(const rendering::AbstractPointCloudProvider& pointcloud_provider);
	void updateModuleChoices(uint id);
	void onAddButtonClicked(int postprocessor_index);
	void onRemoveButtonClicked();
	void onPostprocessorRemoved(int index);
};

}