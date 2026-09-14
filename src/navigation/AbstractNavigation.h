#pragma once

#include "Camera.h"

namespace sahara::navigation
{

class AbstractNavigation
{

public:
	AbstractNavigation(Camera* camera) noexcept;
	virtual ~AbstractNavigation() = default;

	virtual void update(float move_speed, float look_speed) = 0;

protected:
	Camera* m_camera;
};

}