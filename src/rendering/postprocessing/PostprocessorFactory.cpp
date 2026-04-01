#include "PostprocessorFactory.h"

#include "EyeDomeLightingPostprocessor.h"
#include "HoleFillingPostprocessor.h"

namespace sahara::rendering
{

std::unique_ptr<AbstractPostprocessor> PostprocessorFactory::createPostprocessor(PostprocessorType type, OpenGLContext* opengl_context, navigation::Camera* camera)
{
	switch (type)
	{
		case PostprocessorType::EyeDomeLighting:
			return std::make_unique<EyeDomeLightingPostprocessor>(opengl_context, camera);
		case PostprocessorType::HoleFilling:
			return std::make_unique<HoleFillingPostprocessor>(opengl_context);
		default:
			assert(false && "Unknown postprocessor type");
			return nullptr;
	}
}

}
