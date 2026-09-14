#pragma once

#include "AbstractOOCPointCloudProvider.h"

namespace sahara::rendering
{

// this point cloud provider stores an LoD data structure and maintains a buffer of fixed size for the currently relevant relevant point cloud chunks.
// the chunks are loaded from disk based on the virtual viewpoint
class OOCSinglePointCloudProvider : public AbstractOOCPointCloudProvider
{
public:
	OOCSinglePointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context, navigation::Camera* camera);
	~OOCSinglePointCloudProvider();

	PointCloudProviderType type() const noexcept override;

private:
	void loadOctree(const std::filesystem::path& filepath);
};
}