#pragma once

#include "navigation/CameraAnimationPath.h"

#include <QJsonObject>
#include <memory>

namespace sahara::io
{

bool exportCameraPathToJson(const navigation::CameraAnimationPath& camera_path, const QString& filepath);
std::unique_ptr<navigation::CameraAnimationPath> importCameraPathFromJson(const QString& filepath);

}