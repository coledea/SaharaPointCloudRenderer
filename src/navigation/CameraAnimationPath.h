#pragma once

#include "Camera.h"

#include <QEasingCurve>
#include <QJsonObject>
#include <QVector3D>
#include <memory>
#include <string>
#include <vector>

namespace sahara::navigation
{

struct CameraAnimationKeyframe
{
	CameraSpecification camera;
	int duration;
	QEasingCurve easing;
};

typedef std::vector<CameraAnimationKeyframe> CameraAnimationPath;

}