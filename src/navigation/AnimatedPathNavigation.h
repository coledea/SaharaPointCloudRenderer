#pragma once

#include "AbstractNavigation.h"
#include "CameraAnimationPath.h"

#include <QSequentialAnimationGroup>

namespace sahara::navigation
{

class AnimatedPathNavigation : public QObject, public AbstractNavigation
{
	Q_OBJECT

public:
	AnimatedPathNavigation(Camera* camera);

	void onMouseMoveLeftButton(QPoint move, float move_speed, float look_speed) override;
	void onMouseMoveRightButton(QPoint move, float move_speed, float look_speed) override;
	void onWheelMove(float delta, float move_speed, float look_speed) override;

	void startAnimation(const CameraAnimationPath& camera_path, bool forward, bool loop);
	void pauseAnimation();
	void stopAnimation();

signals:
	void animationStopped();

private:
	void onAnimationStateChanged(QAbstractAnimation::State new_state, QAbstractAnimation::State old_state);

	QSequentialAnimationGroup m_animation;
};

}