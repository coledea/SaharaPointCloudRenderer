#include "RendererSettingsPanel.h"

#include "rendering/RendererModuleTypes.h"
#include "ui_RendererSettingsPanel.h"

namespace sahara::ui
{

RendererSettingsPanel::RendererSettingsPanel(rendering::Scene* scene, QWidget* parent)
	: QWidget(parent)
	, m_ui(std::make_unique<Ui::RendererSettingsPanel>())
	, m_scene(scene)
	, m_active_pointcloud_id(0)
	, m_pointcloud_provider_parameters(rendering::RendererModule::PointCloudProvider, scene, false, this)
	, m_rasterizer_parameters(rendering::RendererModule::Rasterizer, scene, true, this)
	, m_colorizer_parameters(rendering::RendererModule::Colorizer, scene, true, this)
	, m_postprocessor_parameters(scene, this)
{
	m_ui->setupUi(this);
	m_ui->TW_ParameterTabs->insertTab(0, &m_pointcloud_provider_parameters, "Data");
	m_ui->TW_ParameterTabs->insertTab(1, &m_rasterizer_parameters, "Rasterization");
	m_ui->TW_ParameterTabs->insertTab(2, &m_colorizer_parameters, "Shading");
	m_ui->TW_ParameterTabs->insertTab(3, &m_postprocessor_parameters, "Postprocessing");
}

RendererSettingsPanel::~RendererSettingsPanel()
{
	// We need a destructor here, as Ui::ParametersPanel was forward-declared
}

void RendererSettingsPanel::setActivePointCloud(uint id)
{
	m_pointcloud_provider_parameters.setActivePointCloud(id);
	m_rasterizer_parameters.setActivePointCloud(id);
	m_colorizer_parameters.setActivePointCloud(id);
	m_postprocessor_parameters.setActivePointCloud(id);
}

}