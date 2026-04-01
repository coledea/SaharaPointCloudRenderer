#include "CameraPathPanel.h"

#include "io/CameraPathIO.h"
#include "ui_CameraPathPanel.h"
#include "utils/EasingCurveIcons.h"
#include "utils/Profiler.h"

#include <QFileDialog>
#include <QMetaEnum>
#include <QPainter>
#include <QPainterPath>

namespace sahara::ui
{

CameraPathPanel::CameraPathPanel(navigation::NavigationHandler* navigation_handler, navigation::Camera* camera, QWidget* parent)
	: QWidget(parent)
	, m_ui(std::make_unique<Ui::CameraPathPanel>())
	, m_navigation_handler(navigation_handler)
	, m_camera(camera)
	, m_camera_path(std::make_unique<navigation::CameraAnimationPath>())
{
	m_ui->setupUi(this);

	populateComboboxWithEasingCurves();

	connect(m_ui->TB_ItemAdd, &QToolButton::clicked, this, &CameraPathPanel::addCameraPathKeyframe);
	connect(m_ui->TB_ItemRemove, &QToolButton::clicked, this, &CameraPathPanel::removeCameraPathKeyframe);
	connect(m_ui->TB_Clear, &QToolButton::clicked, this, &CameraPathPanel::clearCameraPath);
	connect(m_ui->TB_PathImport, &QToolButton::clicked, this, &CameraPathPanel::importCameraPath);
	connect(m_ui->TB_PathExport, &QToolButton::clicked, this, &CameraPathPanel::exportCameraPath);

	connect(m_ui->LW_CameraPath, &QListWidget::itemSelectionChanged, this, &CameraPathPanel::onKeyframeSelectionChanged);
	connect(m_ui->LW_CameraPath, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) { jumpToKeyframe(m_ui->LW_CameraPath->row(item)); });

	connect(m_ui->TB_cameraJumpToFirst, &QToolButton::clicked, this, &CameraPathPanel::jumpToFirstKeyframe);
	connect(m_ui->TB_cameraJumpToLast, &QToolButton::clicked, this, &CameraPathPanel::jumpToLastKeyframe);
	connect(m_ui->TB_cameraJumpToNext, &QToolButton::clicked, this, &CameraPathPanel::jumpToNextKeyframe);
	connect(m_ui->TB_cameraJumpToPrev, &QToolButton::clicked, this, &CameraPathPanel::jumpToPreviousKeyframe);
	connect(m_ui->TB_cameraPause, &QToolButton::clicked, this, &CameraPathPanel::pausePlayback);
	connect(m_ui->TB_cameraPlay, &QToolButton::clicked, this, &CameraPathPanel::playForward);
	connect(m_ui->TB_cameraStop, &QToolButton::clicked, this, &CameraPathPanel::stopPlayback);
	connect(m_ui->TB_cameraPlayBackwards, &QToolButton::clicked, this, &CameraPathPanel::playBackwards);

	connect(m_ui->SB_Duration, &QSpinBox::valueChanged, this, &CameraPathPanel::onKeyframeDurationChanged);
	connect(m_ui->CB_Easing, &QComboBox::currentIndexChanged, this, &CameraPathPanel::onKeyframeEasingChanged);

	connect(m_navigation_handler, &navigation::NavigationHandler::cameraPathAnimationStopped, this, &CameraPathPanel::onCameraPathAnimationStopped);
}

CameraPathPanel::~CameraPathPanel()
{
	// We need a destructor here, as Ui::CameraPathPanel was forward-declared
}

void CameraPathPanel::populateComboboxWithEasingCurves()
{
	for (int i = 0; i < QEasingCurve::NCurveTypes - 3; i++)
	{
		auto easing_type_name = QEasingCurve::staticMetaObject.enumerator(QEasingCurve::staticMetaObject.indexOfEnumerator("Type")).valueToKey(i);
		m_ui->CB_Easing->insertItem(i, utils::easingCurveIcon(i), easing_type_name);
	}
}

void CameraPathPanel::addCameraPathKeyframe()
{
	// check if the new keyframe would be a duplicate of the last keyframe
	if (!m_camera_path->empty())
	{
		const navigation::CameraSpecification last_keyframe_camera = m_camera_path->back().camera;
		const navigation::CameraSpecification new_keyframe = m_camera->cameraSpecifications();
		if (last_keyframe_camera.eye == new_keyframe.eye && last_keyframe_camera.center == new_keyframe.center && last_keyframe_camera.up == new_keyframe.up && last_keyframe_camera.fov == new_keyframe.fov)
		{
			return;
		}
	}

	navigation::CameraAnimationKeyframe keyframe;
	keyframe.camera = m_camera->cameraSpecifications();
	keyframe.duration = 1000;
	keyframe.easing = QEasingCurve::Linear;

	m_ui->LW_CameraPath->addItem(new QListWidgetItem("Keyframe " + QString::number(m_camera_path->size()), m_ui->LW_CameraPath));
	m_camera_path->push_back(keyframe);
	m_ui->LW_CameraPath->setCurrentRow(m_ui->LW_CameraPath->count() - 1);
	updateButtonsEnabled();
}

void CameraPathPanel::removeCameraPathKeyframe()
{
	if (isItemSelected())
	{
		const int index = m_ui->LW_CameraPath->currentRow();
		delete m_ui->LW_CameraPath->takeItem(index);
		m_camera_path->erase(m_camera_path->begin() + index);
	}

	for (int i = 0; i < m_ui->LW_CameraPath->count(); i++)
	{
		m_ui->LW_CameraPath->item(i)->setText("Keyframe " + QString::number(i));
	}

	updateButtonsEnabled();
}

void CameraPathPanel::clearCameraPath()
{
	m_ui->LW_CameraPath->clearSelection();
	m_camera_path->clear();
	m_ui->LW_CameraPath->clear();
	updateButtonsEnabled();
}

void CameraPathPanel::importCameraPath()
{
	QString camera_path_name = QFileDialog::getOpenFileName(this, tr("Import Camera Animation Path"), "./", tr("Camera Path (*.camerapath.json)"));
	if (camera_path_name.isEmpty())
	{
		return;
	}

	std::unique_ptr<navigation::CameraAnimationPath> camera_path(io::importCameraPathFromJson(camera_path_name));
	if (camera_path->empty())
	{
		qWarning() << "Loaded camera animation path is empty";
		return;
	}

	clearCameraPath();
	m_camera_path = std::move(camera_path);
	for (int i = 0; i < m_camera_path->size(); i++)
	{
		m_ui->LW_CameraPath->addItem(new QListWidgetItem("Keyframe " + QString::number(i), m_ui->LW_CameraPath));
	}

	updateButtonsEnabled();
}

void CameraPathPanel::exportCameraPath()
{
	QString camera_path_name = QFileDialog::getSaveFileName(this, tr("Export Camera Animation Path"), "./", tr("Camera Path (*.camerapath.json)"));
	if (camera_path_name.isEmpty())
	{
		return;
	}

	io::exportCameraPathToJson(*m_camera_path, camera_path_name);
}

void CameraPathPanel::playForward()
{
	m_navigation_handler->activateAnimatedPathNavigation(*m_camera_path, true, m_ui->TB_cameraLoop->isChecked());
	updateButtonsEnabled();
	if (m_ui->TB_profiling->isChecked())
	{
		utils::global_profiler.startSession("./profiler_results", 1000); // TODO: Make output folder configurable
	}
}

void CameraPathPanel::playBackwards()
{
	m_navigation_handler->activateAnimatedPathNavigation(*m_camera_path, false, m_ui->TB_cameraLoop->isChecked());
	updateButtonsEnabled();
}

void CameraPathPanel::pausePlayback()
{
	m_navigation_handler->pauseAnimatedPathNavigation();
}

void CameraPathPanel::stopPlayback()
{
	m_navigation_handler->stopAnimatedPathNavigation();
	updateButtonsEnabled();
}

void CameraPathPanel::jumpToPreviousKeyframe()
{
	if (!isItemSelected())
	{
		return;
	}

	m_ui->LW_CameraPath->setCurrentRow((m_ui->LW_CameraPath->currentRow() + (m_ui->LW_CameraPath->count() - 1)) % m_ui->LW_CameraPath->count());
	jumpToKeyframe(m_ui->LW_CameraPath->currentRow());
}

void CameraPathPanel::jumpToNextKeyframe()
{
	if (!isItemSelected())
	{
		return;
	}

	m_ui->LW_CameraPath->setCurrentRow((m_ui->LW_CameraPath->currentRow() + 1) % m_ui->LW_CameraPath->count());
	jumpToKeyframe(m_ui->LW_CameraPath->currentRow());
}

void CameraPathPanel::jumpToFirstKeyframe()
{
	if (m_ui->LW_CameraPath->count() == 0)
	{
		return;
	}

	m_ui->LW_CameraPath->setCurrentRow(0);
	jumpToKeyframe(m_ui->LW_CameraPath->currentRow());
}

void CameraPathPanel::jumpToLastKeyframe()
{
	if (m_ui->LW_CameraPath->count() == 0)
	{
		return;
	}

	m_ui->LW_CameraPath->setCurrentRow(m_ui->LW_CameraPath->count() - 1);
	jumpToKeyframe(m_ui->LW_CameraPath->currentRow());
}

void CameraPathPanel::updateButtonsEnabled()
{
	const bool has_items = m_ui->LW_CameraPath->count() > 0;

	if (m_navigation_handler->isAnimatedPathNavigationActive())
	{
		m_ui->TB_ItemAdd->setEnabled(false);
		m_ui->TB_ItemRemove->setEnabled(false);
		m_ui->TB_Clear->setEnabled(false);
		m_ui->TB_PathImport->setEnabled(false);
		m_ui->GB_KeyFrame->setEnabled(false);
		m_ui->TB_cameraJumpToFirst->setEnabled(false);
		m_ui->TB_cameraJumpToLast->setEnabled(false);
		m_ui->TB_cameraJumpToNext->setEnabled(false);
		m_ui->TB_cameraJumpToPrev->setEnabled(false);
		m_ui->TB_cameraLoop->setEnabled(false);
	}
	else
	{
		m_ui->TB_ItemAdd->setEnabled(true);
		m_ui->TB_ItemRemove->setEnabled(isItemSelected());
		m_ui->TB_Clear->setEnabled(has_items);
		m_ui->TB_PathImport->setEnabled(true);
		m_ui->GB_KeyFrame->setEnabled(true);
		m_ui->TB_cameraJumpToFirst->setEnabled(true);
		m_ui->TB_cameraJumpToLast->setEnabled(true);
		m_ui->TB_cameraJumpToNext->setEnabled(true);
		m_ui->TB_cameraJumpToPrev->setEnabled(true);
		m_ui->TB_cameraLoop->setEnabled(true);
	}

	m_ui->TB_PathExport->setEnabled(has_items);
}

void CameraPathPanel::jumpToKeyframe(int index)
{
	assert(index >= 0 && index < m_camera_path->size());
	m_camera->setCameraSpecification(m_camera_path->at(index).camera);
}

void CameraPathPanel::onKeyframeSelectionChanged()
{
	updateButtonsEnabled();

	if (!isItemSelected())
	{
		return;
	}

	const navigation::CameraAnimationKeyframe& keyframe = m_camera_path->at(m_ui->LW_CameraPath->currentRow());
	m_ui->SB_Duration->setValue(keyframe.duration);
	m_ui->CB_Easing->setCurrentIndex(keyframe.easing.type());
}

void CameraPathPanel::onKeyframeDurationChanged(int duration)
{
	if (!isItemSelected())
	{
		return;
	}

	m_camera_path->at(m_ui->LW_CameraPath->currentRow()).duration = duration;
}

void CameraPathPanel::onKeyframeEasingChanged(int easing_index)
{
	if (!isItemSelected())
	{
		return;
	}

	m_camera_path->at(m_ui->LW_CameraPath->currentRow()).easing = QEasingCurve(static_cast<QEasingCurve::Type>(easing_index));
}

void CameraPathPanel::onCameraPathAnimationStopped()
{
	updateButtonsEnabled();
	if (m_ui->TB_profiling->isChecked())
	{
		utils::global_profiler.endSession();
	}
}

bool CameraPathPanel::isItemSelected()
{
	return m_ui->LW_CameraPath->currentItem() != nullptr;
}

}