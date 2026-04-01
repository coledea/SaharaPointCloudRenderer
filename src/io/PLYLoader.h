#pragma once

#include "geometry/StaticPointCloud.h"

#include <tinyply/tinyply.h>

namespace sahara::io
{
class PLYLoader
{
public:
	PLYLoader();
	std::unique_ptr<geometry::StaticPointCloud> load(const std::string& filepath);

private:
	std::unique_ptr<tinyply::PlyFile> m_file;
	std::unique_ptr<std::istream> m_filestream;
	std::vector<tinyply::PlyProperty> m_properties;
	std::vector<std::string> m_requested_properties;

	void openFile(const std::string& filepath);
	void loadProperties();
	std::shared_ptr<tinyply::PlyData> requestPropertiesIfAvailable(std::vector<std::string> property_names);
	void addCustomAttributes(geometry::StaticPointCloud* pointcloud, const std::unordered_map<std::string, std::shared_ptr<tinyply::PlyData>>& custom_attributes);

	template <typename T_original, typename T_attribute>
	void addPointCloudAttribute(geometry::StaticPointCloud* pointcloud, std::shared_ptr<tinyply::PlyData> data, geometry::AttributeSemantic attribute_semantic, const QString& name);
};

}

#include "PLYLoader.inl"