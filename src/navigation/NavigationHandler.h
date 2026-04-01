#pragma once

#include "AnimatedPathNavigation.h"
#include "Camera.h"
#include "FirstPersonNavigation.h"
#include "OrbitalNavigation.h"

#include <QWheelEvent>

namespace sahara::navigation
{

class NavigationHandler : public QObject
{
	Q_OBJECT

public:
	NavigationHandler(Camera* camera);

	void mousePressEvent(QPoint pos, Qt::MouseButton button);
	void mouseReleaseEvent(QPoint pos, Qt::MouseButton button);
	void mouseMoveEvent(QPoint pos);
	void wheelEvent(const QWheelEvent& e);

	bool isAnimatedPathNavigationActive();

signals:
	void cameraPathAnimationStopped();

	// for informing the camera settings panel when the camera settings changed due to camera path animations
	void moveSpeedChanged(float move_speed);
	void lookSpeedChanged(float look_speed);

public slots:
	void setMoveSpeed(float move_speed) noexcept;
	void setLookSpeed(float look_speed) noexcept;
	void activateFirstPersonNavigation();
	void activateOrbitalNavigation();

	void activateAnimatedPathNavigation(const CameraAnimationPath& camera_path, bool forward, bool loop);
	void pauseAnimatedPathNavigation();
	void stopAnimatedPathNavigation();

	void onAnimatedPathNavigationStopped();

private:
	FirstPersonNavigation m_first_person_navigation;
	OrbitalNavigation m_orbital_navigation;
	AnimatedPathNavigation m_animated_path_navigation;

	AbstractNavigation* m_current_navigation;
	AbstractNavigation* m_next_navigation;

	bool m_left_mouse_button_pressed;
	bool m_right_mouse_button_pressed;
	QPoint m_last_mouse_position;
	float m_move_speed;
	float m_look_speed;
};

}