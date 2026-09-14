#include "ScenePanel.h"

#include "ui/InspectPointCloudDialog.h"
#include "ui_ScenePanel.h"

#include <QMenu>

namespace sahara::ui
{

ScenePanel::ScenePanel(rendering::Scene* scene, QWidget* parent)
	: QWidget(parent)
	, m_ui(std::make_unique<Ui::ScenePanel>())
	, m_scene(scene)
{
	m_ui->setupUi(this);

	m_ui->LW_SceneObjects->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_ui->LW_SceneObjects, &QListWidget::customContextMenuRequested, this, &ScenePanel::openElementContextMenu);
	connect(m_ui->LW_SceneObjects, &QListWidget::itemSelectionChanged, this, &ScenePanel::onItemSelectionChanged);
	connect(m_ui->LW_SceneObjects, &QListWidget::doubleClicked, this, &ScenePanel::focusPointCloud);
	connect(m_scene, &rendering::Scene::pointCloudAdded, this, &ScenePanel::onPointCloudAdded);
}

ScenePanel::~ScenePanel()
{
	// We need a destructor here, as Ui::ScenePanel was forward-declared
}

std::optional<uint> ScenePanel::activePointCloudID() const
{
	if (auto item = dynamic_cast<ScenePanelItem*>(m_ui->LW_SceneObjects->currentItem()))
	{
		return item->pointCloudID();
	}

	return std::nullopt;
}

QString ScenePanel::activePointCloudName() const
{
	if (auto item = dynamic_cast<ScenePanelItem*>(m_ui->LW_SceneObjects->currentItem()))
	{
		return item->pointCloudName();
	}

	return QString();
}

void ScenePanel::onPointCloudAdded(const rendering::AbstractPointCloudProvider& pointcloud_provider)
{
	uint id = pointcloud_provider.id();

	auto item = new ScenePanelItem(id, pointcloud_provider.name());
	auto item_widget = new ScenePanelItemWidget(pointcloud_provider.name());
	connect(item_widget, &ScenePanelItemWidget::visibilityToggled, [this, id](bool visible) { m_scene->setPointCloudVisible(id, visible); });

	m_ui->LW_SceneObjects->addItem(item);
	m_ui->LW_SceneObjects->setItemWidget(item, item_widget);
	m_ui->LW_SceneObjects->setCurrentItem(item);
}

void ScenePanel::openElementContextMenu(const QPoint& position)
{
	auto index = m_ui->LW_SceneObjects->indexAt(position);
	if (index.isValid())
	{
		QMenu context_menu;

		auto action_remove = new QAction("Remove");
		connect(action_remove, &QAction::triggered, [this, &index]([[maybe_unused]] bool checked) { removePointCloud(index); });
		context_menu.addAction(action_remove);

		auto action_inspect = new QAction("Show Details");
		connect(action_inspect, &QAction::triggered, [this, &index]([[maybe_unused]] bool checked) { showPointCloudDetails(index); });
		context_menu.addAction(action_inspect);

		auto action_focus = new QAction("Focus");
		connect(action_focus, &QAction::triggered, [this, &index]([[maybe_unused]] bool checked) { focusPointCloud(index); });
		context_menu.addAction(action_focus);

		context_menu.exec(QCursor::pos());
	}
}

void ScenePanel::focusPointCloud(const QModelIndex& index)
{
	auto item = dynamic_cast<ScenePanelItem*>(m_ui->LW_SceneObjects->itemFromIndex(index));
	if (item != nullptr)
	{
		m_ui->LW_SceneObjects->setCurrentItem(item);
		emit pointCloudFocusRequested(item->pointCloudID());
	}
}

void ScenePanel::removePointCloud(const QModelIndex& index)
{
	auto item = dynamic_cast<ScenePanelItem*>(m_ui->LW_SceneObjects->itemFromIndex(index));
	m_scene->removePointCloud(item->pointCloudID());
	delete m_ui->LW_SceneObjects->takeItem(m_ui->LW_SceneObjects->row(item));
}

void ScenePanel::showPointCloudDetails(const QModelIndex& index)
{
	auto item = dynamic_cast<ScenePanelItem*>(m_ui->LW_SceneObjects->itemFromIndex(index));
	InspectPointCloudDialog dialog(m_scene->pointCloud(item->pointCloudID()), this);
	dialog.exec();
}

void ScenePanel::onItemSelectionChanged()
{
	if (auto item = dynamic_cast<ScenePanelItem*>(m_ui->LW_SceneObjects->currentItem()))
	{
		emit activePointCloudChanged(item->pointCloudID());
	}
}

}
