#include "StaticPointCloudFactory.h"

#include "io/PLYLoader.h"

#include <cmath>

namespace sahara::geometry::StaticPointCloudFactory
{

std::unique_ptr<StaticPointCloud> loadFromPointCloudFile(const std::filesystem::path& filepath)
{
	const auto extension = filepath.extension();
	if (extension == ".ply")
	{
		io::PLYLoader ply_loader;
		return ply_loader.load(filepath.string());
	}
	return std::make_unique<geometry::StaticPointCloud>();
}

std::unique_ptr<StaticPointCloud> loadFromMeshFile(const std::filesystem::path& filepath)
{
	// TODO
	// Load mesh
	// Sample mesh
	return std::make_unique<geometry::StaticPointCloud>();
}

std::unique_ptr<StaticPointCloud> createSyntheticPointCloud(uint num_points)
{
	// For testing purposes: creates a cone-shaped point cloud.
	auto pointcloud = std::make_unique<geometry::StaticPointCloud>();
	AttributeDataVector<QVector3D> position_data(num_points);
	AttributeDataVector<Color> color_data(num_points);
	AttributeDataVector<uint> id_data(num_points);
	float x, y = -1.0f, z = 0.0, amplitude = 0.0f;
	float z_increment = -2.0f / (num_points / 360.0f);
	constexpr double angle_step = M_PI / 180.0f;

	for (size_t point_count = 0; point_count < num_points; point_count++)
	{
		x = amplitude * std::cos(point_count * angle_step);
		y = amplitude * std::sin(point_count * angle_step);
		position_data[point_count] = QVector3D(x, y, z);
		color_data[point_count] = { static_cast<uint8_t>(std::abs(x)), static_cast<uint8_t>(std::abs(y)), static_cast<uint8_t>(std::abs(z)) };
		id_data[point_count] = point_count;
		if (point_count % 360 == 0)
		{
			z += z_increment;
			amplitude += z_increment;
		}
	}

	auto position_attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<QVector3D>>("position", geometry::AttributeSemantic::Position, position_data);
	auto position_attribute_data = std::make_unique<geometry::AttributeData<QVector3D>>(std::move(position_data));
	pointcloud->addAttribute(std::move(position_attribute_data), std::move(position_attribute_metadata));

	auto color_attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<Color>>("color", geometry::AttributeSemantic::Color, color_data);
	auto color_attribute_data = std::make_unique<geometry::AttributeData<Color>>(std::move(color_data));
	pointcloud->addAttribute(std::move(color_attribute_data), std::move(color_attribute_metadata));

	auto id_attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<uint>>("id", geometry::AttributeSemantic::ID, id_data);
	auto id_attribute_data = std::make_unique<geometry::AttributeData<uint>>(std::move(id_data));
	pointcloud->addAttribute(std::move(id_attribute_data), std::move(id_attribute_metadata));

	return pointcloud;
}

}