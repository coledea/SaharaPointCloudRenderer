#include "ParametersPanel.h"

#include "ui/parameterWidgets/ParameterWidgetFactory.h"
#include "ui_ParametersPanel.h"

#include <QComboBox>
#include <QStandardItemModel>

namespace sahara::ui
{

ParametersPanel::ParametersPanel(rendering::RendererModule renderer_module, rendering::Scene* scene, bool allow_module_selection, QWidget* parent)
	: QWidget(parent)
	, m_ui(std::make_unique<Ui::ParametersPanel>())
	, m_renderer_module(renderer_module)
	, m_scene(scene)
	, m_active_pointcloud_id(0)
	, m_allow_module_selection(allow_module_selection)
{
	m_ui->setupUi(this);
	m_ui->CB_approachSelection->setDisabled(true);
	for (int i = 0; i < rendering::RendererModuleNames.at(m_renderer_module).size(); i++)
	{
		m_ui->CB_approachSelection->insertItem(i, rendering::RendererModuleNames.at(m_renderer_module).at(i));
	}

	connect(m_ui->CB_approachSelection, &QComboBox::currentIndexChanged, m_scene, [this](int index) { 
		m_scene->renderer(m_active_pointcloud_id).changeModule(m_renderer_module, index); 
		recreateParameterWidgets(m_active_pointcloud_id); });
	connect(scene, &rendering::Scene::pointCloudAdded, this, [&](const rendering::AbstractPointCloudProvider& pointcloud_provider) { onPointcloudAdded(pointcloud_provider.id()); });
	connect(scene, &rendering::Scene::pointCloudRemoved, this, &ParametersPanel::onPointcloudRemoved);
}

ParametersPanel::~ParametersPanel()
{
	// We need a destructor here, as Ui::ColorizerParametersPanel was forward-declared
}

void ParametersPanel::setActivePointCloud(uint id)
{
	m_ui->parametersScrollArea->takeWidget();

	const QSignalBlocker blocker(m_ui->CB_approachSelection);
	m_ui->CB_approachSelection->setCurrentIndex(m_scene->renderer(id).moduleType(m_renderer_module));
	if (m_allow_module_selection)
	{
		m_ui->CB_approachSelection->setEnabled(true);
	}

	if (m_parameters.find(id) != m_parameters.end())
	{
		m_ui->parametersScrollArea->setWidget(m_parameters[id].get());
		m_active_pointcloud_id = id;
		updateModuleChoicesForCurrentPointcloud();
	}
}

void ParametersPanel::onPointcloudRemoved(uint id)
{
	if (m_parameters.find(id) == m_parameters.end())
	{
		return;
	}

	if (m_active_pointcloud_id == id)
	{
		m_ui->parametersScrollArea->takeWidget();
		m_ui->CB_approachSelection->setDisabled(true);
	}

	m_parameters.erase(id);
}

void ParametersPanel::onPointcloudAdded(uint id)
{
	m_parameters[id] = std::make_unique<MultipleParametersWidget>(this);
	recreateParameterWidgets(id);
	setActivePointCloud(id);
}

void ParametersPanel::recreateParameterWidgets(uint id)
{
	m_parameters[id]->removeAllParameterWidgets();
	for (auto parameter : m_scene->renderer(id).moduleParameters(m_renderer_module))
	{
		m_parameters[id]->addParameterWidget(ParameterWidgetFactory::createParameterWidget(parameter));
	}
}

void ParametersPanel::updateModuleChoicesForCurrentPointcloud()
{
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_ui->CB_approachSelection->model());
	for (int i = 0; i < rendering::RendererModuleNames.at(m_renderer_module).size(); i++)
	{
		if (m_scene->renderer(m_active_pointcloud_id).isModuleTypeSupported(m_renderer_module, i))
		{
			model->item(i)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		}
		else
		{
			model->item(i)->setFlags(Qt::NoItemFlags);
		}
	}
}

}