#include "InputEventCache.h"

InputEventCache::InputEventCache()
	: m_mouse_press_occurred(false)
	, m_mouse_release_occurred(false)
	, m_mouse_move_occurred(false)
	, m_wheel_occurred(false)
	, m_key_press_occurred(false)
	, m_key_release_occurred(false)
	, m_resize_occurred(false)
{
}

void InputEventCache::recordMousePressEvent(QPointF pos, Qt::MouseButton button)
{
	m_mouse_press_occurred = true;
	m_mouse_position = pos;
	m_pressed_button = button;
	m_pressed_buttons.insert_or_assign(button, true);
}

void InputEventCache::recordMouseReleaseEvent(QPointF pos, Qt::MouseButton button)
{
	m_mouse_release_occurred = true;
	m_mouse_position = pos;
	m_released_button = button;
	m_pressed_buttons.insert_or_assign(button, false);
}

void InputEventCache::recordMouseMoveEvent(QPointF pos)
{
	m_mouse_move_occurred = true;
	m_mouse_movement += pos - m_mouse_position;
	m_mouse_position = pos;
}

void InputEventCache::recordWheelEvent(QWheelEvent* wheelEvent)
{
	m_wheel_occurred = true;
	m_wheel_angle_delta = wheelEvent->angleDelta();

	if (wheelEvent->hasPixelDelta())
	{
		m_wheel_pixel_delta = wheelEvent->pixelDelta();
	}
	else
	{
		m_wheel_pixel_delta.reset();
	}
}

void InputEventCache::recordKeyPressEvent(QKeyEvent* keyPressEvent)
{
	m_key_press_occurred = true;
	m_pressed_keys.insert_or_assign(keyPressEvent->key(), true);
}

void InputEventCache::recordKeyReleaseEvent(QKeyEvent* keyReleaseEvent)
{
	m_key_release_occurred = true;
	m_pressed_keys.insert_or_assign(keyReleaseEvent->key(), false);
}

void InputEventCache::recordResizeEvent(const QPoint scaledSize)
{
	m_resize_occurred = true;
	m_framebuffer_size = scaledSize;
}

bool InputEventCache::didMousePressOccur() const noexcept
{
	return m_mouse_press_occurred;
}

bool InputEventCache::didMouseReleaseOccur() const noexcept
{
	return m_mouse_release_occurred;
}

bool InputEventCache::didMouseMoveOccur() const noexcept
{
	return m_mouse_move_occurred;
}

bool InputEventCache::didWheelOccur() const noexcept
{
	return m_wheel_occurred;
}

bool InputEventCache::didKeyPressOccur() const noexcept
{
	return m_key_press_occurred;
}

bool InputEventCache::didKeyReleaseOccur() const noexcept
{
	return m_key_release_occurred;
}

bool InputEventCache::didResizeOccur() const noexcept
{
	return m_resize_occurred;
}

QPointF InputEventCache::getMousePosition() const
{
	return m_mouse_position;
}

QPointF InputEventCache::getMouseMovement() const
{
	return m_mouse_movement;
}

Qt::MouseButton InputEventCache::getPressedButton() const
{
	return m_pressed_button;
}

Qt::MouseButton InputEventCache::getReleasedButton() const
{
	return m_released_button;
}

bool InputEventCache::isKeyPressed(Qt::Key key) const
{
	auto iterator = m_pressed_keys.find(key);
	if (iterator != m_pressed_keys.end())
	{
		return iterator->second;
	}
	return false;
}

bool InputEventCache::isMouseButtonPressed(Qt::MouseButton button) const
{
	auto iterator = m_pressed_buttons.find(button);
	if (iterator != m_pressed_buttons.end())
	{
		return iterator->second;
	}
	return false;
}

std::optional<QPoint> InputEventCache::getWheelPixelDelta() const
{
	return m_wheel_pixel_delta;
}

QPoint InputEventCache::getWheelAngleDelta() const
{
	return m_wheel_angle_delta;
}

QPoint InputEventCache::getFramebufferSize() const
{
	return m_framebuffer_size;
}

void InputEventCache::reset()
{
	m_mouse_press_occurred = false;
	m_mouse_release_occurred = false;
	m_mouse_move_occurred = false;
	m_wheel_occurred = false;
	m_key_press_occurred = false;
	m_key_release_occurred = false;
	m_resize_occurred = false;
	m_mouse_movement = QPointF(0.0f, 0.0f);
}

void InputEventCache::clear()
{
	reset();
	m_pressed_keys.clear();
	m_pressed_buttons.clear();
}