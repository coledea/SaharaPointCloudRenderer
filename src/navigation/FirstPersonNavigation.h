#pragma once

#include "AbstractNavigation.h"

namespace sahara::navigation
{

class FirstPersonNavigation : public AbstractNavigation
{
public:
	FirstPersonNavigation(Camera* camera);

	void onMouseMoveLeftButton(QPointF move, float move_speed, float look_speed) override;
	void onMouseMoveRightButton(QPointF move, float move_speed, float look_speed) override;
	void onWheelMove(float delta, float move_speed, float look_speed) override;

private:
};

}