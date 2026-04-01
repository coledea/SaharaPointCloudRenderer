#pragma once

#include "CameraAnimationPath.h"

#include <QAbstractAnimation>

namespace sahara::navigation
{

class CameraAnimationStep : public QAbstractAnimation
{
public:
	CameraAnimationStep(Camera* camera, const CameraAnimationKeyframe& source, const CameraAnimationKeyframe& target);

	int duration() const noexcept override;
	void updateCurrentTime(int current_time) override;

private:
	CameraAnimationKeyframe m_source;
	CameraAnimationKeyframe m_target;
	QQuaternion m_source_orientation;
	QQuaternion m_target_orientation;
	Camera* m_camera;
};

}