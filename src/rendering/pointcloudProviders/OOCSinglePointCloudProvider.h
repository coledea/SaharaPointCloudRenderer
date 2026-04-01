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
	std::unique_ptr<EnumParameter> m_priority_projection_function_parameter;
	std::unique_ptr<RangeParameter<float>> m_priority_projection_factor_parameter;

	std::unique_ptr<EnumParameter> m_priority_distance_function_parameter;
	std::unique_ptr<RangeParameter<float>> m_priority_distance_factor_parameter;

	std::unique_ptr<RangeParameter<float>> m_priority_centrality_factor_parameter;

	std::unique_ptr<Parameter<bool>> m_priority_use_parent_child_parameter;
	std::unique_ptr<RangeParameter<float>> m_priority_parent_child_factor_parameter;

	std::unique_ptr<Parameter<bool>> m_priority_use_recency_parameter;

	void initializePriorityFunctionParameters();
	void onPriorityProjectionFunctionChanged();
	void onPriorityDistanceFunctionChanged();
	void onPriorityProjectionFactorChanged();
	void onPriorityDistanceFactorChanged();
	void onPriorityCentralityFactorChanged();
	void onPriorityUseRecencyChanged();
};
}