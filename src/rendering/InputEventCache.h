#pragma once

#include <QWheelEvent>

class InputEventCache
{
public:
	InputEventCache();

	void recordMousePressEvent(QPointF pos, Qt::MouseButton button);
	void recordMouseReleaseEvent(QPointF pos, Qt::MouseButton button);
	void recordMouseMoveEvent(QPointF pos);
	void recordWheelEvent(QWheelEvent* wheelEvent);
	void recordKeyPressEvent(QKeyEvent* keyPressEvent);
	void recordKeyReleaseEvent(QKeyEvent* keyReleaseEvent);
	void recordResizeEvent(QPoint scaledSize);

	bool didMousePressOccur() const noexcept;
	bool didMouseReleaseOccur() const noexcept;
	bool didMouseMoveOccur() const noexcept;
	bool didWheelOccur() const noexcept;
	bool didKeyPressOccur() const noexcept;
	bool didKeyReleaseOccur() const noexcept;
	bool didResizeOccur() const noexcept;

	QPointF getMousePosition() const;
	QPointF getMouseMovement() const;
	Qt::MouseButton getPressedButton() const;
	Qt::MouseButton getReleasedButton() const;

	bool isKeyPressed(Qt::Key key) const;
	bool isMouseButtonPressed(Qt::MouseButton button) const;
	std::optional<QPoint> getWheelPixelDelta() const;
	QPoint getWheelAngleDelta() const;
	QPoint getFramebufferSize() const;

	void reset();

private:
	bool m_mouse_press_occurred;
	bool m_mouse_release_occurred;
	bool m_mouse_move_occurred;
	bool m_wheel_occurred;
	bool m_key_press_occurred;
	bool m_key_release_occurred;
	bool m_resize_occurred;

	QPointF m_mouse_position;
	QPointF m_mouse_movement;
	std::unordered_map<int, bool> m_pressed_keys;
	std::unordered_map<int, bool> m_pressed_buttons;
	Qt::MouseButton m_pressed_button;
	Qt::MouseButton m_released_button;
	std::optional<QPoint> m_wheel_pixel_delta;
	QPoint m_wheel_angle_delta;
	QPoint m_framebuffer_size;
};
