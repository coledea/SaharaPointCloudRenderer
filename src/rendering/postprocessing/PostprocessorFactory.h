#pragma once

#include "AbstractPostprocessor.h"
#include "navigation/Camera.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

namespace sahara::rendering
{

class PostprocessorFactory
{
public:
	static std::unique_ptr<AbstractPostprocessor> createPostprocessor(PostprocessorType type, OpenGLContext* opengl_context, navigation::Camera* camera);
};

}