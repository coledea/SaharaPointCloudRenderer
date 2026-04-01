#include "RasterizerFactory.h"

#include "OOCPointPrimitiveRasterizer.h"
#include "PointPrimitiveRasterizer.h"

namespace sahara::rendering
{

std::unique_ptr<AbstractRasterizer> RasterizerFactory::createRasterizer(RasterizerType type, OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera)
{
	switch (type)
	{
		case RasterizerType::PointPrimitiveRasterizer:
			return std::make_unique<PointPrimitiveRasterizer>(opengl_context, provider, camera);
		case RasterizerType::OOCPointPrimitiveRasterizer:
			return std::make_unique<OOCPointPrimitiveRasterizer>(opengl_context, provider, camera);
		default:
			assert(false && "Unknown rasterizer type");
			return nullptr;
	}
}

}
