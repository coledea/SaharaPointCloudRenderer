#include "CameraSettingsPanel.h"

#include "ui_CameraSettingsPanel.h"

namespace sahara::ui
{

CameraSettingsPanel::CameraSettingsPanel(navigation::NavigationHandler* navigation_handler, navigation::Camera* camera, QWidget* parent)
	: QWidget(parent)
	, m_ui(std::make_unique<Ui::CameraSettingsPanel>())
	, m_navigation_handler(navigation_handler)
	, m_camera(camera)
{
	m_ui->setupUi(this);

	connect(m_ui->DSB_NearPlane, &QDoubleSpinBox::valueChanged, m_camera, &navigation::Camera::setNearPlane);
	connect(m_ui->DSB_FarPlane, &QDoubleSpinBox::valueChanged, m_camera, &navigation::Camera::setFarPlane);
	connect(m_ui->SB_FieldOfView, &QSpinBox::valueChanged, this, [this](int fov) { m_camera->setFov(static_cast<float>(fov)); });
	connect(m_ui->SB_MoveSpeed, &QSpinBox::valueChanged, m_navigation_handler, &navigation::NavigationHandler::setMoveSpeed);
	connect(m_ui->SB_LookSpeed, &QSpinBox::valueChanged, m_navigation_handler, &navigation::NavigationHandler::setLookSpeed);
	connect(m_ui->B_FirstPersonCamera, &QToolButton::toggled, this, &CameraSettingsPanel::toggleFirstPersonNavigation);
	connect(m_ui->B_OrbitalCamera, &QToolButton::toggled, this, &CameraSettingsPanel::toggleOrbitalNavigation);

	connect(m_camera, &navigation::Camera::fovChanged, this, [this](float fov) {
		QSignalBlocker(m_ui->SB_FieldOfView);
		m_ui->SB_FieldOfView->setValue(static_cast<int>(fov)); });

	connect(m_camera, &navigation::Camera::farPlaneChanged, this, [this](float farPlane) {
		QSignalBlocker(m_ui->DSB_FarPlane);
		m_ui->DSB_FarPlane->setValue(farPlane); });

	connect(m_camera, &navigation::Camera::nearPlaneChanged, this, [this](float nearPlane) {
		QSignalBlocker(m_ui->DSB_NearPlane);
		m_ui->DSB_NearPlane->setValue(nearPlane); });

	connect(m_navigation_handler, &navigation::NavigationHandler::moveSpeedChanged, this, [this](float move_speed) {
		QSignalBlocker(m_ui->SB_MoveSpeed);
		m_ui->SB_MoveSpeed->setValue(static_cast<int>(move_speed));
	});

	connect(m_navigation_handler, &navigation::NavigationHandler::lookSpeedChanged, this, [this](float look_speed) {
		QSignalBlocker(m_ui->SB_LookSpeed);
		m_ui->SB_LookSpeed->setValue(static_cast<int>(look_speed));
	});
}

CameraSettingsPanel::~CameraSettingsPanel()
{
	// We need a destructor here, as Ui::CameraSettingsPanel was forward-declared
}

void CameraSettingsPanel::toggleFirstPersonNavigation(bool active)
{
	active ? m_navigation_handler->activateFirstPersonNavigation() : m_navigation_handler->activateOrbitalNavigation();
}

void CameraSettingsPanel::toggleOrbitalNavigation(bool active)
{
	active ? m_navigation_handler->activateOrbitalNavigation() : m_navigation_handler->activateFirstPersonNavigation();
}

}