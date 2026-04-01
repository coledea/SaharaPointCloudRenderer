#pragma once

#include "AbstractPointCloudProvider.h"
#include "navigation/NavigationHandler.h"

#include <filesystem>

namespace sahara::rendering
{

class PointCloudProviderFactory
{
public:
	static std::unique_ptr<AbstractPointCloudProvider> createPointCloudProvider(const std::filesystem::path& filepath, OpenGLContext* opengl_context, navigation::Camera* camera, navigation::NavigationHandler* navigation_handler);
};

}