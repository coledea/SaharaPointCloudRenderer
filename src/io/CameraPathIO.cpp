#include "CameraPathIO.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

namespace sahara::io
{

const QString JSON_KEY_CAMERA_EYE = "eye";
const QString JSON_KEY_CAMERA_CENTER = "center";
const QString JSON_KEY_CAMERA_UP = "up";
const QString JSON_KEY_CAMERA_NEARPLANE = "nearplane";
const QString JSON_KEY_CAMERA_FARPLANE = "farplane";
const QString JSON_KEY_CAMERA_FOV = "fov";
const QString JSON_KEY_CAMERA_DURATION = "duration";
const QString JSON_KEY_CAMERA_EASING = "easing";

QVector3D fromJsonObject(const QJsonObject& object)
{
	return QVector3D(object["x"].toDouble(), object["y"].toDouble(), object["z"].toDouble());
}

QJsonObject toJsonObject(const QVector3D& vector)
{
	QJsonObject object;
	object["x"] = vector.x();
	object["y"] = vector.y();
	object["z"] = vector.z();
	return object;
}

bool exportCameraPathToJson(const navigation::CameraAnimationPath& camera_path, const QString& filepath)
{
	QFile file(filepath);

	if (!file.open(QIODevice::WriteOnly))
	{
		qWarning() << "Could not open file: " << filepath;
		return false;
	}

	QJsonArray camera_path_array;
	for (const auto& keyframe : camera_path)
	{
		QJsonObject keyframeObject;
		keyframeObject[JSON_KEY_CAMERA_EYE] = toJsonObject(keyframe.camera.eye);
		keyframeObject[JSON_KEY_CAMERA_CENTER] = toJsonObject(keyframe.camera.center);
		keyframeObject[JSON_KEY_CAMERA_UP] = toJsonObject(keyframe.camera.up);
		keyframeObject[JSON_KEY_CAMERA_NEARPLANE] = keyframe.camera.near_plane;
		keyframeObject[JSON_KEY_CAMERA_FARPLANE] = keyframe.camera.far_plane;
		keyframeObject[JSON_KEY_CAMERA_FOV] = keyframe.camera.fov;
		keyframeObject[JSON_KEY_CAMERA_DURATION] = keyframe.duration;
		keyframeObject[JSON_KEY_CAMERA_EASING] = keyframe.easing.type();
		camera_path_array.append(keyframeObject);
	}

	file.write(QJsonDocument(camera_path_array).toJson());
	file.close();
	return true;
}

std::unique_ptr<navigation::CameraAnimationPath> importCameraPathFromJson(const QString& filepath)
{
	auto camera_path = std::make_unique<navigation::CameraAnimationPath>();
	QFile file(filepath);

	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Could not open file: " << filepath;
		return camera_path;
	}

	QByteArray file_bytes = file.readAll();
	file.close();

	QJsonParseError parse_error;
	QJsonDocument json_doc = QJsonDocument::fromJson(file_bytes, &parse_error);

	if (parse_error.error != QJsonParseError::NoError)
	{
		qWarning() << "Could not parse JSON at " << parse_error.offset << ":" << parse_error.errorString();
		return camera_path;
	}

	QJsonArray camera_path_array = json_doc.array();

	if (camera_path_array.empty())
	{
		qWarning() << "Loaded camera path is empty!";
		return camera_path;
	}

	for (const auto& entry : camera_path_array)
	{
		QJsonObject keyframe_object = entry.toObject();

		navigation::CameraAnimationKeyframe keyframe;
		keyframe.camera.eye = fromJsonObject(keyframe_object[JSON_KEY_CAMERA_EYE].toObject());
		keyframe.camera.center = fromJsonObject(keyframe_object[JSON_KEY_CAMERA_CENTER].toObject());
		keyframe.camera.up = fromJsonObject(keyframe_object[JSON_KEY_CAMERA_UP].toObject());
		keyframe.camera.near_plane = keyframe_object[JSON_KEY_CAMERA_NEARPLANE].toVariant().toFloat();
		keyframe.camera.far_plane = keyframe_object[JSON_KEY_CAMERA_FARPLANE].toVariant().toFloat();
		keyframe.camera.fov = keyframe_object[JSON_KEY_CAMERA_FOV].toVariant().toFloat();
		keyframe.duration = keyframe_object[JSON_KEY_CAMERA_DURATION].toInt();
		keyframe.easing = QEasingCurve(static_cast<QEasingCurve::Type>(keyframe_object[JSON_KEY_CAMERA_EASING].toInt()));

		camera_path->push_back(keyframe);
	}

	return camera_path;
}

}