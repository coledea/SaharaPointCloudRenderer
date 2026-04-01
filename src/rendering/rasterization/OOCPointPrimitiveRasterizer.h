#pragma once

#include "PointPrimitiveRasterizer.h"

namespace sahara::rendering
{

class OOCPointPrimitiveRasterizer : public PointPrimitiveRasterizer
{
	Q_OBJECT

public:
	OOCPointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	~OOCPointPrimitiveRasterizer();

	void reloadShaderSpecificationsFromDisk() override;
	void run() override;

	RasterizerType type() const noexcept override;

private:
	void initializeVAO() override;
};

}