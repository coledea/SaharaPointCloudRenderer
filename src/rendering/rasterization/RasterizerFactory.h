#pragma once

#include "AbstractRasterizer.h"

namespace sahara::rendering
{

class RasterizerFactory
{
public:
	static std::unique_ptr<AbstractRasterizer> createRasterizer(RasterizerType type, OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera);
};

}