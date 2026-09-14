#pragma once

#include "AbstractNavigation.h"

namespace sahara::navigation
{

class OrbitalNavigation : public AbstractNavigation
{
public:
	OrbitalNavigation(Camera* camera);

	void update(float move_speed, float look_speed) override;

private:
	void onMouseMoveLeftButton(QPointF move, float move_speed);
	void onMouseMoveRightButton(QPointF move, float look_speed);
	void onWheelMove(float delta, float move_speed);
};

}