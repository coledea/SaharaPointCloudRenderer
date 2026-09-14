#include "OOCMultiTemporalPointCloudProvider.h"

#include "utils/Profiler.h"

#include <QFile>
#include <QFileInfo>
#include <QtConcurrent>

namespace sahara::rendering
{

OOCMultiTemporalPointCloudProvider::OOCMultiTemporalPointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context, navigation::Camera* camera)
	: AbstractOOCPointCloudProvider(filepath.string(), opengl_context, camera)
	, m_number_of_timestamps(0)
{
	loadOctree(filepath);
	initializeParameters();
	initializeMultiTemporalParameters();
}

OOCMultiTemporalPointCloudProvider::~OOCMultiTemporalPointCloudProvider()
{
}

void OOCMultiTemporalPointCloudProvider::loadOctree(const std::filesystem::path& filepath)
{
	// ############## READ IN META DATA ####################
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Could not open file " << filepath.c_str();
		return;
	}

	const auto metadata_json = QJsonDocument::fromJson(file.readAll());
	file.close();

	std::vector<int> num_nodes_per_timestamp;
	std::vector<geometry::BoundingBox> bounding_box_per_timestamp;

	extractTimestampMetadataFromJSON(metadata_json, num_nodes_per_timestamp, bounding_box_per_timestamp);
	extractAttributeMetadataFromJSON(metadata_json, bounding_box_per_timestamp);

	// ############## CREATE OTHER COMPONENTS ####################
	std::filesystem::path octree_path = filepath;
	octree_path.replace_filename("nodes.bin");
	m_octree = std::make_unique<geometry::MultiTemporalLodOctree>(octree_path, num_nodes_per_timestamp, bounding_box_per_timestamp, m_opengl_context);

	m_chunk_loader = std::make_unique<io::OOCPointCloudLoader>(filepath.parent_path(), m_number_of_points);

	// we keep the metadata of all nodes, even though we only consider and sort a subset for rendering.
	// We could also reconfigure the capacities for priority computation, draw command buffer, etc. on timestamp change
	auto overall_num_nodes = dynamic_cast<geometry::MultiTemporalLodOctree*>(m_octree.get())->overallNumberOfNodes();
	m_draw_command_buffer.setCapacity(overall_num_nodes);
	m_nodes_should_remain_on_cpu.resize(overall_num_nodes);
	m_nodes_should_remain_on_gpu.resize(overall_num_nodes);
}

void OOCMultiTemporalPointCloudProvider::extractTimestampMetadataFromJSON(const QJsonDocument& metadata, std::vector<int>& num_nodes, std::vector<geometry::BoundingBox>& bounding_boxes)
{
	auto timestamps = metadata.object().value("epochs").toArray();
	for (auto timestamp = timestamps.begin(); timestamp != timestamps.end(); timestamp++)
	{
		num_nodes.push_back(timestamp->toObject().value("numNodes").toInt());
		m_number_of_points += timestamp->toObject().value("numPoints").toInt();
		const auto bounding_box_json = timestamp->toObject().value("boundingBox").toObject();
		QVector3D bbox_min(bounding_box_json.value("lx").toDouble(), bounding_box_json.value("ly").toDouble(), bounding_box_json.value("lz").toDouble());
		QVector3D bbox_max(bounding_box_json.value("ux").toDouble(), bounding_box_json.value("uy").toDouble(), bounding_box_json.value("uz").toDouble());
		geometry::BoundingBox bounding_box(bbox_min, bbox_max);
		bounding_boxes.push_back(bounding_box);
	}
	m_number_of_timestamps = static_cast<uint>(num_nodes.size());
}

void OOCMultiTemporalPointCloudProvider::extractAttributeMetadataFromJSON(const QJsonDocument& metadata, const std::vector<geometry::BoundingBox>& timestamp_bounding_boxes)
{
	// calculate overall bbox across all timestamps
	QVector3D position_min{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	QVector3D position_max{ -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };
	for (const auto& bbox : timestamp_bounding_boxes)
	{
		position_min = min(position_min, bbox.minimum());
		position_max = max(position_max, bbox.maximum());
	}

	// create attribute metadata definitions
	auto attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<QVector3D>>("position", geometry::AttributeSemantic::Position, position_min, position_max);
	m_attribute_metadata.emplace(geometry::AttributeSemantic::Position, std::move(attribute_metadata));
	m_available_attributes.emplace(geometry::AttributeSemantic::Position, m_attribute_metadata[geometry::AttributeSemantic::Position].get());

	const auto attributes = metadata.object().value("attributes").toObject();
	for (const auto& key : attributes.keys())
	{
		auto type = attributes.value(key).toString();

		if (key == "color")
		{
			auto attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<geometry::Color>>(key, geometry::AttributeSemantic::Color, geometry::Color(0, 0, 0), geometry::Color(255, 255, 255));
			m_attribute_metadata.emplace(geometry::AttributeSemantic::Color, std::move(attribute_metadata));
			m_available_attributes.emplace(geometry::AttributeSemantic::Color, m_attribute_metadata[geometry::AttributeSemantic::Color].get());
		}
		// TODO: support for other/custom attributes
	}
}

void OOCMultiTemporalPointCloudProvider::initializeMultiTemporalParameters()
{
	m_timestamp_parameter = std::make_unique<RangeParameter<int>>("Timestamp", 0, 0, m_number_of_timestamps - 1, 1);
	connect(m_timestamp_parameter.get(), &RangeParameter<int>::valueChanged, this, &OOCMultiTemporalPointCloudProvider::onTimestampChanged);
	m_parameters.push_back(m_timestamp_parameter.get());
	onTimestampChanged();

	m_timestamp_range_parameter = std::make_unique<RangeParameter<int>>("Timestamp distance for loading/caching", 0, 0, m_number_of_timestamps - 1, 1);
	connect(m_timestamp_range_parameter.get(), &RangeParameter<int>::valueChanged, this, &OOCMultiTemporalPointCloudProvider::onTimestampRangeParameterChanged);
	m_parameters.push_back(m_timestamp_range_parameter.get());
	onTimestampRangeParameterChanged();

	m_timestamp_falloff_parameter = std::make_unique<RangeParameter<float>>("Timestamp Priority Factor", 0.5f, 0.0, 1.0, 0.01); // 0.0 = current timestamp is by far most important (hyperbolic), 0.5 = linear falloff, 1.0 = all timestamps are similarly important
	connect(m_timestamp_falloff_parameter.get(), &RangeParameter<float>::valueChanged, this, &OOCMultiTemporalPointCloudProvider::onTimestampFalloffParameterChanged);
	m_parameters.push_back(m_timestamp_falloff_parameter.get());
	onTimestampFalloffParameterChanged();

	m_animate_timestamps_parameter = std::make_unique<Parameter<bool>>("Animate Timestamps", false);
	connect(m_animate_timestamps_parameter.get(), &Parameter<bool>::valueChanged, this, &OOCMultiTemporalPointCloudProvider::onAnimateTimestampsParameterChanged);
	m_parameters.push_back(m_animate_timestamps_parameter.get());
	onAnimateTimestampsParameterChanged();

	m_timestamp_duration_parameter = std::make_unique<RangeParameter<float>>("Timestamp Duration", 1.0f, 0.001f, 10.0f, 0.01f);
	connect(m_timestamp_duration_parameter.get(), &RangeParameter<float>::valueChanged, this, &OOCMultiTemporalPointCloudProvider::onTimestampDurationParameterChanged);
	m_parameters.push_back(m_timestamp_duration_parameter.get());
}

PointCloudProviderType OOCMultiTemporalPointCloudProvider::type() const noexcept
{
	return PointCloudProviderType::OOCMultiTemporalPointCloudProvider;
}

void OOCMultiTemporalPointCloudProvider::onTimestampChanged()
{
	int timestamp = m_timestamp_parameter->value();
	utils::global_profiler.addMeasurement("timestamp_changes", timestamp);
	dynamic_cast<geometry::MultiTemporalLodOctree*>(m_octree.get())->setTimestamp(timestamp);
}

void OOCMultiTemporalPointCloudProvider::onTimestampRangeParameterChanged()
{
	dynamic_cast<geometry::MultiTemporalLodOctree*>(m_octree.get())->setTimestampDistanceToConsider(m_timestamp_range_parameter->value());
}

void OOCMultiTemporalPointCloudProvider::onTimestampFalloffParameterChanged()
{
	dynamic_cast<geometry::MultiTemporalLodOctree*>(m_octree.get())->setTimestampPriorityFalloff(m_timestamp_falloff_parameter->value());
}

void OOCMultiTemporalPointCloudProvider::onAnimateTimestampsParameterChanged()
{
	if (m_animate_timestamps_parameter->value() == true)
	{
		m_timestamp_animation_timer.start(m_timestamp_duration_parameter->value() * 1000.0f, this);
	}
	else
	{
		m_timestamp_animation_timer.stop();
	}
}

void OOCMultiTemporalPointCloudProvider::onTimestampDurationParameterChanged()
{
	if (m_animate_timestamps_parameter->value() == true)
	{
		m_timestamp_animation_timer.start(m_timestamp_duration_parameter->value() * 1000.0f, this);
	}
}

void OOCMultiTemporalPointCloudProvider::timerEvent(QTimerEvent* e)
{
	if (e->timerId() != m_timestamp_animation_timer.timerId())
	{
		return;
	}
	m_timestamp_parameter->setValue((m_timestamp_parameter->value() + 1) % (m_timestamp_parameter->maximum() + 1));
}
}