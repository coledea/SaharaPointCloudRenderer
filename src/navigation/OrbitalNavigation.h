#pragma once

#include "AbstractNavigation.h"

namespace sahara::navigation
{

class OrbitalNavigation : public AbstractNavigation
{
public:
	OrbitalNavigation(Camera* camera);

	void onMouseMoveLeftButton(QPointF move, float move_speed, float look_speed) override;
	void onMouseMoveRightButton(QPointF move, float move_speed, float look_speed) override;
	void onWheelMove(float delta, float move_speed, float look_speed) override;

private:
};

}