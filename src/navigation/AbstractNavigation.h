#pragma once

#include "Camera.h"

namespace sahara::navigation
{

class AbstractNavigation
{

public:
	AbstractNavigation(Camera* camera) noexcept;
	virtual ~AbstractNavigation() = default;

	virtual void onMouseMoveLeftButton(QPoint move, float move_speed, float look_speed) = 0;
	virtual void onMouseMoveRightButton(QPoint move, float move_speed, float look_speed) = 0;
	virtual void onWheelMove(float delta, float move_speed, float look_speed) = 0;

protected:
	Camera* m_camera;
};

}