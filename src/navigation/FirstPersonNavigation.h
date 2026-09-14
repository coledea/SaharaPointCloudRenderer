#pragma once

#include "AbstractNavigation.h"

namespace sahara::navigation
{

class FirstPersonNavigation : public AbstractNavigation
{
public:
	FirstPersonNavigation(Camera* camera);

	void update(float move_speed, float look_speed) override;

private:
	void onMouseMoveRightButton(QPointF move, float look_speed);
};

}