#pragma once

#include "navigation/Camera.h"
#include "navigation/NavigationHandler.h"

#include <QWidget>

namespace Ui
{
class CameraSettingsPanel;
}

namespace sahara::ui
{

class CameraSettingsPanel : public QWidget
{
	Q_OBJECT

public:
	explicit CameraSettingsPanel(navigation::NavigationHandler* navigation_handler, navigation::Camera* camera, QWidget* parent = nullptr);
	~CameraSettingsPanel();

public slots:

	void toggleFirstPersonNavigation(bool active);
	void toggleOrbitalNavigation(bool active);

private:
	std::unique_ptr<Ui::CameraSettingsPanel> m_ui;
	navigation::NavigationHandler* m_navigation_handler;
	navigation::Camera* m_camera;
};

}