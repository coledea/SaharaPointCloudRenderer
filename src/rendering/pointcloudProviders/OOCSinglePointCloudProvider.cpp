#include "OOCSinglePointCloudProvider.h"

#include "utils/Profiler.h"

#include <QFile>
#include <QtConcurrent>

namespace sahara::rendering
{

OOCSinglePointCloudProvider::OOCSinglePointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context, navigation::Camera* camera)
	: AbstractOOCPointCloudProvider(filepath.string(), opengl_context, camera)
{
	loadOctree(filepath);
	initializeParameters();
}

OOCSinglePointCloudProvider::~OOCSinglePointCloudProvider()
{
}

PointCloudProviderType OOCSinglePointCloudProvider::type() const noexcept
{
	return PointCloudProviderType::OOCSinglePointCloudProvider;
}

void OOCSinglePointCloudProvider::loadOctree(const std::filesystem::path& filepath)
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

	size_t num_nodes = metadata_json.object().value("numNodes").toInt();
	m_number_of_points = metadata_json.object().value("numPoints").toInt();
	// size_t num_voxels = metadata_json.object().value("numVoxels").toInt();
	// m_number_of_points = std::max(m_number_of_points, num_voxels);

	const auto bounding_box_json = metadata_json.object().value("boundingBox").toObject();
	QVector3D bbox_min(bounding_box_json.value("lx").toDouble(), bounding_box_json.value("ly").toDouble(), bounding_box_json.value("lz").toDouble());
	QVector3D bbox_max(bounding_box_json.value("ux").toDouble(), bounding_box_json.value("uy").toDouble(), bounding_box_json.value("uz").toDouble());
	geometry::BoundingBox bounding_box(bbox_min, bbox_max);

	auto metadata = std::make_unique<geometry::TypedAttributeMetadata<QVector3D>>("position", geometry::AttributeSemantic::Position, bounding_box.minimum(), bounding_box.maximum());
	m_attribute_metadata.emplace(geometry::AttributeSemantic::Position, std::move(metadata));
	m_available_attributes.emplace(geometry::AttributeSemantic::Position, m_attribute_metadata[geometry::AttributeSemantic::Position].get());

	const auto attributes = metadata_json.object().value("attributes").toObject();
	for (const auto& key : attributes.keys())
	{
		auto type = attributes.value(key).toString();

		if (key == "color")
		{
			auto metadata = std::make_unique<geometry::TypedAttributeMetadata<geometry::Color>>(key, geometry::AttributeSemantic::Color, geometry::Color(0, 0, 0), geometry::Color(255, 255, 255));
			m_attribute_metadata.emplace(geometry::AttributeSemantic::Color, std::move(metadata));
			m_available_attributes.emplace(geometry::AttributeSemantic::Color, m_attribute_metadata[geometry::AttributeSemantic::Color].get());
		}
		// TODO: support for other/custom attributes
	}

	// ############## CREATE COMPONENTS ####################
	std::filesystem::path octree_path = filepath;
	octree_path.replace_filename("nodes.bin");
	m_octree = std::make_unique<geometry::LodOctree>(octree_path, num_nodes, bounding_box, m_opengl_context);
	dynamic_cast<geometry::LodOctree*>(m_octree.get())->configureHierarchyPriority(true, 1.0);

	m_draw_command_buffer.setCapacity(m_octree->nodes().size());

	m_nodes_should_remain_on_cpu.resize(m_octree->nodes().size());
	m_nodes_should_remain_on_gpu.resize(m_octree->nodes().size());

	m_chunk_loader = std::make_unique<io::OOCPointCloudLoader>(filepath.parent_path(), m_number_of_points);
}

}