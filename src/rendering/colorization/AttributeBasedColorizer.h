#pragma once

#include "AbstractColorizer.h"
#include "navigation/Camera.h"
#include "rendering/parameters/EnumParameter.h"
#include "rendering/parameters/RangeParameter.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

namespace sahara::rendering
{

class AttributeBasedColorizer : public AbstractColorizer
{
	Q_OBJECT

public:
	AttributeBasedColorizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) noexcept;

	~AttributeBasedColorizer();

	void reloadShaderSpecificationsFromDisk() override;
	void setCompiledShaderProgram(QOpenGLShaderProgram* shader_program) override;

	virtual ColorizerType type() const noexcept override;

private:
	std::unique_ptr<EnumParameter> m_attribute_selection_parameter;
	std::unique_ptr<EnumParameter> m_color_scale_parameter;
	std::unique_ptr<RangeParameter<float>> m_original_color_factor_parameter;
	AbstractPointCloudProvider* m_pointcloud_provider;
	navigation::Camera* m_camera;

	QString m_shader_code;

	void onAttributeSelectionChanged();
	void onColorScaleChanged();
	void onOriginalColorFactorChanged();
	void onCameraFarPlaneChanged(float far_plane);
	void onCameraNearPlaneChanged(float near_plane);
};

}