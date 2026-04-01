#include "ColorizerFactory.h"

#include "AttributeBasedColorizer.h"
#include "DefaultColorizer.h"
#include "SingleColorColorizer.h"

namespace sahara::rendering
{

std::unique_ptr<AbstractColorizer> ColorizerFactory::createColorizer(ColorizerType type, OpenGLContext* opengl_context, rendering::AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera)
{
	switch (type)
	{
		case ColorizerType::None:
			return std::make_unique<DefaultColorizer>();
			break;
		case ColorizerType::SingleColor:
			return std::make_unique<SingleColorColorizer>(opengl_context);
			break;
		case ColorizerType::AttributeBased:
			return std::make_unique<AttributeBasedColorizer>(opengl_context, pointcloud_provider, camera);
			break;
		default:
			assert(false && "Unknown colorizer type");
			return nullptr;
	}
}

}
