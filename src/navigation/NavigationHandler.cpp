#include "NavigationHandler.h"

namespace sahara::navigation
{

NavigationHandler::NavigationHandler(Camera* camera)
	: m_first_person_navigation(camera)
	, m_orbital_navigation(camera)
	, m_animated_path_navigation(camera)
	, m_current_navigation(&m_orbital_navigation)
	, m_next_navigation(&m_orbital_navigation)
	, m_last_mouse_position(0, 0)
	, m_left_mouse_button_pressed(false)
	, m_right_mouse_button_pressed(false)
	, m_move_speed(50.0f)
	, m_look_speed(10.0f)
{
	connect(&m_animated_path_navigation, &AnimatedPathNavigation::animationStopped, this, &NavigationHandler::onAnimatedPathNavigationStopped);
}

void NavigationHandler::setMoveSpeed(float move_speed) noexcept
{
	m_move_speed = move_speed;
	emit moveSpeedChanged(move_speed);
}

void NavigationHandler::setLookSpeed(float look_speed) noexcept
{
	m_look_speed = look_speed;
	emit lookSpeedChanged(look_speed);
}

void NavigationHandler::activateFirstPersonNavigation()
{
	// Wait for animation to finish before setting the new navigation strategy
	if (isAnimatedPathNavigationActive())
	{
		m_next_navigation = &m_first_person_navigation;
	}
	else
	{
		m_current_navigation = &m_first_person_navigation;
	}
}

void NavigationHandler::activateOrbitalNavigation()
{
	// Wait for animation to finish before setting the new navigation strategy
	if (isAnimatedPathNavigationActive())
	{
		m_next_navigation = &m_orbital_navigation;
	}
	else
	{
		m_current_navigation = &m_orbital_navigation;
	}
}

void NavigationHandler::activateAnimatedPathNavigation(const CameraAnimationPath& camera_path, bool forward, bool loop)
{
	if (!isAnimatedPathNavigationActive())
	{
		m_next_navigation = m_current_navigation;
	}

	m_current_navigation = &m_animated_path_navigation;
	m_animated_path_navigation.startAnimation(camera_path, forward, loop);
}

void NavigationHandler::pauseAnimatedPathNavigation()
{
	m_animated_path_navigation.pauseAnimation();
}

void NavigationHandler::stopAnimatedPathNavigation()
{
	m_animated_path_navigation.stopAnimation();
	m_current_navigation = m_next_navigation;
}

void NavigationHandler::onAnimatedPathNavigationStopped()
{
	m_current_navigation = m_next_navigation;
	emit cameraPathAnimationStopped();
}

bool NavigationHandler::isAnimatedPathNavigationActive()
{
	return dynamic_cast<AnimatedPathNavigation*>(m_current_navigation) != nullptr;
}

void NavigationHandler::mousePressEvent(QPoint pos, Qt::MouseButton button)
{
	if (button == Qt::LeftButton)
	{
		m_left_mouse_button_pressed = true;
	}
	else if (button == Qt::RightButton)
	{
		m_right_mouse_button_pressed = true;
	}

	m_last_mouse_position = pos;
}

void NavigationHandler::mouseReleaseEvent(QPoint pos, Qt::MouseButton button)
{
	if (button == Qt::LeftButton)
	{
		m_left_mouse_button_pressed = false;
	}
	else if (button == Qt::RightButton)
	{
		m_right_mouse_button_pressed = false;
	}

	m_last_mouse_position = pos;
}

void NavigationHandler::mouseMoveEvent(QPoint pos)
{
	if (m_left_mouse_button_pressed)
	{
		m_current_navigation->onMouseMoveLeftButton(pos - m_last_mouse_position, m_move_speed, m_look_speed);
	}
	if (m_right_mouse_button_pressed)
	{
		m_current_navigation->onMouseMoveRightButton(pos - m_last_mouse_position, m_move_speed, m_look_speed);
	}

	m_last_mouse_position = pos;
}

void NavigationHandler::wheelEvent(const QWheelEvent& e)
{
	float delta = 0.0f;
	if (e.hasPixelDelta())
	{
		delta = e.pixelDelta().y();
	}
	else
	{
		delta = static_cast<float>(e.angleDelta().y() / 120);
	}
	m_current_navigation->onWheelMove(delta, m_move_speed, m_look_speed);
}

}