#include "AnimatedPathNavigation.h"

#include "CameraAnimationStep.h"

namespace sahara::navigation
{

AnimatedPathNavigation::AnimatedPathNavigation(Camera* camera)
	: AbstractNavigation(camera)
{
	connect(&m_animation, &QAbstractAnimation::stateChanged, this, &AnimatedPathNavigation::onAnimationStateChanged);
}

void AnimatedPathNavigation::onMouseMoveLeftButton([[maybe_unused]] QPoint move, [[maybe_unused]] float move_speed, [[maybe_unused]] float look_speed)
{
}

void AnimatedPathNavigation::onMouseMoveRightButton([[maybe_unused]] QPoint move, [[maybe_unused]] float move_speed, [[maybe_unused]] float look_speed)
{
}

void AnimatedPathNavigation::onWheelMove([[maybe_unused]] float delta, [[maybe_unused]] float move_speed, [[maybe_unused]] float look_speed)
{
}

void AnimatedPathNavigation::startAnimation(const CameraAnimationPath& camera_path, bool forward, bool loop)
{
	m_animation.setDirection(forward ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
	m_animation.setLoopCount(loop ? -1 : 1);

	if (m_animation.state() == QAbstractAnimation::Paused)
	{
		m_animation.resume();
	}
	else
	{
		m_animation.clear();
		for (size_t i = 0; i < camera_path.size() - 1; i++)
		{
			m_animation.addAnimation(new CameraAnimationStep(m_camera, camera_path.at(i), camera_path.at(i + 1)));
		}

		if (loop && camera_path.size() > 1)
		{
			m_animation.addAnimation(new CameraAnimationStep(m_camera, camera_path.back(), camera_path.front()));
		}

		m_animation.start();
	}
}

void AnimatedPathNavigation::pauseAnimation()
{
	m_animation.pause();
}

void AnimatedPathNavigation::stopAnimation()
{
	m_animation.stop();
}

void AnimatedPathNavigation::onAnimationStateChanged(QAbstractAnimation::State new_state, [[maybe_unused]] QAbstractAnimation::State old_state)
{
	if (new_state == QAbstractAnimation::State::Stopped)
	{
		emit animationStopped();
	}
}

}
