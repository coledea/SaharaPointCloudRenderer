#pragma once

#include "navigation/Camera.h"
#include "navigation/NavigationHandler.h"

#include <QWidget>

namespace Ui
{
class CameraPathPanel;
}

namespace sahara::ui
{

class CameraPathPanel : public QWidget
{
	Q_OBJECT

public:
	explicit CameraPathPanel(navigation::NavigationHandler* navigation_handler, navigation::Camera* camera, QWidget* parent = nullptr);
	~CameraPathPanel();

public slots:

	void addCameraPathKeyframe();
	void removeCameraPathKeyframe();
	void clearCameraPath();
	void importCameraPath();
	void exportCameraPath();

	void playForward();
	void playBackwards();
	void pausePlayback();
	void stopPlayback();

	void jumpToPreviousKeyframe();
	void jumpToNextKeyframe();
	void jumpToFirstKeyframe();
	void jumpToLastKeyframe();

private:
	void populateComboboxWithEasingCurves();
	void updateButtonsEnabled();
	void jumpToKeyframe(int index);
	void onKeyframeSelectionChanged();
	void onKeyframeDurationChanged(int duration);
	void onKeyframeEasingChanged(int easing_index);
	void onCameraPathAnimationStopped();

	bool isItemSelected();

	std::unique_ptr<Ui::CameraPathPanel> m_ui;

	navigation::NavigationHandler* m_navigation_handler;
	navigation::Camera* m_camera;
	std::unique_ptr<navigation::CameraAnimationPath> m_camera_path;
};

}