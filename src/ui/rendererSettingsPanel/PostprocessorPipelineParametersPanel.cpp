#include "PostprocessorPipelineParametersPanel.h"

#include "ui/parameterWidgets/ParameterWidgetFactory.h"
#include "ui_PostprocessorPipelineParametersPanel.h"

#include <QToolButton>

namespace sahara::ui
{

PostprocessorPipelineParametersPanel::PostprocessorPipelineParametersPanel(rendering::Scene* scene, QWidget* parent)
	: QWidget(parent)
	, m_ui(std::make_unique<Ui::PostprocessorPipelineParametersPanel>())
	, m_scene(scene)
	, m_active_pointcloud_id(0)
{
	m_ui->setupUi(this);

	m_ui->TB_Add->setDisabled(true);
	m_ui->TB_Add->setMenu(&m_add_postprocessor_menu);

	m_ui->TB_Remove->setDisabled(true);
	connect(m_ui->TB_Remove, &QToolButton::clicked, this, &PostprocessorPipelineParametersPanel::onRemoveButtonClicked);

	connect(scene, &rendering::Scene::pointCloudAdded, this, &PostprocessorPipelineParametersPanel::onPointcloudAdded);
	connect(scene, &rendering::Scene::pointCloudRemoved, this, &PostprocessorPipelineParametersPanel::onPointcloudRemoved);
	connect(scene, &rendering::Scene::rendererModulesChanged, this, &PostprocessorPipelineParametersPanel::updateModuleChoices);
}

PostprocessorPipelineParametersPanel::~PostprocessorPipelineParametersPanel()
{
	// We need a destructor here, as Ui::PostprocessorParametersPanel was forward-declared
}

void PostprocessorPipelineParametersPanel::setActivePointCloud(uint id)
{
	m_ui->pipelineScrollArea->takeWidget();

	if (m_parameters.find(id) != m_parameters.end())
	{
		m_active_pointcloud_id = id;
		m_ui->pipelineScrollArea->setWidget(m_parameters[m_active_pointcloud_id].get());
		m_ui->TB_Add->setEnabled(true);
		updateModuleChoices(id);
		m_ui->TB_Remove->setDisabled(m_scene->renderer(m_active_pointcloud_id).numberOfPostprocessors() == 0);
		connect(&m_scene->renderer(m_active_pointcloud_id), &rendering::Renderer::postprocessorRemoved, this, &PostprocessorPipelineParametersPanel::onPostprocessorRemoved);
	}
}

void PostprocessorPipelineParametersPanel::onPointcloudRemoved(uint id)
{
	if (m_parameters.find(id) == m_parameters.end())
	{
		return;
	}
	m_parameters.erase(id);
	m_ui->TB_Add->setDisabled(true);
}

void PostprocessorPipelineParametersPanel::updateModuleChoices(uint id)
{
	m_add_postprocessor_menu.clear();
	for (int i = 0; i < rendering::RendererModuleNames.at(rendering::RendererModule::Postprocessor).size(); i++)
	{
		auto new_action = new QAction(rendering::RendererModuleNames.at(rendering::RendererModule::Postprocessor).at(i));
		connect(new_action, &QAction::triggered, [this, i](bool /*checked*/) { onAddButtonClicked(i); });
		new_action->setDisabled(!m_scene->renderer(m_active_pointcloud_id).isModuleTypeSupported(rendering::RendererModule::Postprocessor, i));
		m_add_postprocessor_menu.addAction(new_action);
	}
}

void PostprocessorPipelineParametersPanel::onPointcloudAdded(const rendering::AbstractPointCloudProvider& pointcloud_provider)
{
	const auto id = pointcloud_provider.id();
	m_parameters.emplace(id, std::make_unique<PostprocessorPipelineParametersWidget>(this));
	setActivePointCloud(id);
}

void PostprocessorPipelineParametersPanel::onAddButtonClicked(int postprocessor_index)
{
	using namespace rendering;
	auto& renderer = m_scene->renderer(m_active_pointcloud_id);
	renderer.appendPostprocessor(static_cast<PostprocessorType>(postprocessor_index));
	m_parameters[m_active_pointcloud_id]->addPostprocessorParameters(RendererModuleNames.at(RendererModule::Postprocessor).at(postprocessor_index), renderer.lastPostprocessorParameters());

	m_ui->TB_Remove->setEnabled(true);
}

void PostprocessorPipelineParametersPanel::onRemoveButtonClicked()
{
	m_scene->renderer(m_active_pointcloud_id).removePostprocessor(m_parameters[m_active_pointcloud_id]->currentIndex());
}

void PostprocessorPipelineParametersPanel::onPostprocessorRemoved(int index)
{
	m_parameters[m_active_pointcloud_id]->removePostprocessorParameters(index);
	m_ui->TB_Remove->setDisabled(m_scene->renderer(m_active_pointcloud_id).numberOfPostprocessors() == 0);
}

}