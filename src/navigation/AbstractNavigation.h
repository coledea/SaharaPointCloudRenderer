#pragma once

#include "Camera.h"

namespace sahara::navigation
{

class AbstractNavigation
{

public:
	AbstractNavigation(Camera* camera) noexcept;
	virtual ~AbstractNavigation() = default;

	virtual void onMouseMoveLeftButton(QPointF move, float move_speed, float look_speed) = 0;
	virtual void onMouseMoveRightButton(QPointF move, float move_speed, float look_speed) = 0;
	virtual void onWheelMove(float delta, float move_speed, float look_speed) = 0;

protected:
	Camera* m_camera;
};

}