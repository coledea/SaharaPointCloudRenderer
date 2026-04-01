#pragma once

#include "AbstractColorizer.h"
#include "navigation/Camera.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

namespace sahara::rendering
{

class ColorizerFactory
{
public:
	static std::unique_ptr<AbstractColorizer> createColorizer(ColorizerType type, OpenGLContext* opengl_context, rendering::AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera);
};

}