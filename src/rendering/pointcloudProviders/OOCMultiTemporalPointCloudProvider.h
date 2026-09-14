#pragma once

#include "AbstractOOCPointCloudProvider.h"
#include "GPUBufferManager.h"
#include "MappedOpenGLBuffer.h"
#include "geometry/MultiTemporalLodOctree.h"
#include "geometry/NodePriorityComputationMultiTemporal.h"
#include "navigation/CameraAnimationPath.h"
#include "rendering/parameters/EnumParameter.h"
#include "rendering/parameters/RangeParameter.h"

#include <QBasicTimer>
#include <QRandomGenerator>
#include <bitset>
#include <filesystem>

namespace sahara::rendering
{

// this point cloud provider stores an LoD data structure and maintains a buffer of fixed size for the currently relevant point cloud chunks.
// the chunks are loaded from disk based on the virtual viewpoint
class OOCMultiTemporalPointCloudProvider : public AbstractOOCPointCloudProvider
{
public:
	OOCMultiTemporalPointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context, navigation::Camera* camera);
	~OOCMultiTemporalPointCloudProvider();

	PointCloudProviderType type() const noexcept override;

private:
	int m_number_of_timestamps;
	QBasicTimer m_timestamp_animation_timer;

	// timestamp selection/priority parameters
	std::unique_ptr<RangeParameter<int>> m_timestamp_parameter;
	std::unique_ptr<RangeParameter<int>> m_timestamp_range_parameter;
	std::unique_ptr<RangeParameter<float>> m_timestamp_falloff_parameter;

	// timestamp animation parameters
	std::unique_ptr<Parameter<bool>> m_animate_timestamps_parameter;
	std::unique_ptr<RangeParameter<float>> m_timestamp_duration_parameter;

	void loadOctree(const std::filesystem::path& filepath);
	void extractTimestampMetadataFromJSON(const QJsonDocument& metadata, std::vector<int>& num_nodes, std::vector<geometry::BoundingBox>& bounding_boxes);
	void extractAttributeMetadataFromJSON(const QJsonDocument& metadata, const std::vector<geometry::BoundingBox>& timestamp_bounding_boxes);
	void initializeMultiTemporalParameters();
	void onTimestampChanged();
	void onTimestampRangeParameterChanged();
	void onTimestampFalloffParameterChanged();
	void onAnimateTimestampsParameterChanged();
	void onTimestampDurationParameterChanged();
	void timerEvent(QTimerEvent* e) override;
};
}