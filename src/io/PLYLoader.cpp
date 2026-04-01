#include "PLYLoader.h"

#define TINYPLY_IMPLEMENTATION
#include "geometry/Color.h"
#include "utils/MemoryStream.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <tinyply/tinyply.h>

namespace sahara::io
{

PLYLoader::PLYLoader()
	: m_file(std::make_unique<tinyply::PlyFile>())
{
}

void PLYLoader::openFile(const std::string& filepath)
{
	const auto file_size_bytes = std::filesystem::file_size(filepath);
	std::vector<uint8_t> byte_buffer(file_size_bytes);

	// We pre-load the file if it's smaller than approx. 1GB.
	if (file_size_bytes < 1.0e9)
	{
		std::ifstream file(filepath, std::ios::binary);
		if (file.is_open())
		{
			file.read(reinterpret_cast<char*>(byte_buffer.data()), file_size_bytes);
		}
		else
		{
			throw std::runtime_error("could not open binary ifstream to path " + filepath);
		}
		m_filestream = std::make_unique<utils::MemoryStream>(std::move(byte_buffer));
	}
	else
	{
		m_filestream = std::make_unique<std::ifstream>(filepath, std::ios::binary);
	}

	if (!m_filestream || m_filestream->fail())
	{
		throw std::runtime_error("file_stream failed to open " + filepath);
	}
	m_file->parse_header(*m_filestream);
}

void PLYLoader::loadProperties()
{
	auto element_index = tinyply::find_element("vertex", m_file->get_elements());
	if (element_index == -1)
	{
		std::cout << "No vertex element found in PLY file." << std::endl;
		return;
	}
	m_properties = m_file->get_elements().at(static_cast<size_t>(element_index)).properties;
}

std::shared_ptr<tinyply::PlyData> PLYLoader::requestPropertiesIfAvailable(std::vector<std::string> property_names)
{
	for (const auto& property : property_names)
	{
		if (tinyply::find_property(property, m_properties) == -1)
		{
			return nullptr;
		}
	}
	m_requested_properties.insert(m_requested_properties.end(), property_names.begin(), property_names.end());
	return m_file->request_properties_from_element("vertex", property_names);
}

void PLYLoader::addCustomAttributes(geometry::StaticPointCloud* pointcloud, const std::unordered_map<std::string, std::shared_ptr<tinyply::PlyData>>& custom_attributes)
{
	int custom_attribute_slot = static_cast<int>(geometry::AttributeSemantic::Custom0);
	for (const auto& entry : custom_attributes)
	{
		switch (entry.second->t)
		{
			case tinyply::Type::FLOAT32:
				addPointCloudAttribute<float, float>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::FLOAT64:
				addPointCloudAttribute<double, float>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::INT32:
				addPointCloudAttribute<int, int>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::INT16:
				addPointCloudAttribute<int16_t, int>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::INT8:
				addPointCloudAttribute<int8_t, int>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::UINT32:
				addPointCloudAttribute<uint, uint>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::UINT16:
				addPointCloudAttribute<uint16_t, uint>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			case tinyply::Type::UINT8:
				addPointCloudAttribute<uint8_t, uint>(pointcloud, entry.second, static_cast<geometry::AttributeSemantic>(custom_attribute_slot), entry.first.c_str());
				break;
			default:
				break;
		}

		custom_attribute_slot++;
		if (custom_attribute_slot >= 11)
		{
			std::cout << "We currently support only 11 custom attributes at the same time." << std::endl;
			break;
		}
	}
}

std::unique_ptr<geometry::StaticPointCloud> PLYLoader::load(const std::string& filepath)
{
	m_properties.clear();
	m_requested_properties.clear();

	auto pointcloud = std::make_unique<geometry::StaticPointCloud>();

	try
	{
		openFile(filepath);
		loadProperties();

		// request common properties
		auto coords = requestPropertiesIfAvailable({ "x", "y", "z" });
		auto colors = requestPropertiesIfAvailable({ "r", "g", "b" });
		if (!colors)
		{
			colors = requestPropertiesIfAvailable({ "red", "green", "blue" });
		}
		auto normals = requestPropertiesIfAvailable({ "nx", "ny", "nz" });

		// also load uncommon properties as custom attributes
		std::unordered_map<std::string, std::shared_ptr<tinyply::PlyData>> custom_attributes;
		for (const auto& property : m_properties)
		{
			// don't request already requested attributes twice
			if (std::find(m_requested_properties.begin(), m_requested_properties.end(), property.name) != m_requested_properties.end())
			{
				continue;
			}
			custom_attributes[property.name] = m_file->request_properties_from_element("vertex", { property.name });
		}

		m_file->read(*m_filestream);

		// add loaded properties as attributes to the point cloud
		if (coords)
		{
			addPointCloudAttribute<QVector3D, QVector3D>(pointcloud.get(), coords, geometry::AttributeSemantic::Position, "position");
		}

		if (normals)
		{
			addPointCloudAttribute<QVector3D, QVector3D>(pointcloud.get(), normals, geometry::AttributeSemantic::Normal, "normal");
		}

		if (colors)
		{
			if (colors->t == tinyply::Type::UINT8)
			{
				addPointCloudAttribute<geometry::Color, geometry::Color>(pointcloud.get(), colors, geometry::AttributeSemantic::Color, "color");
			}
			else
			{
				geometry::AttributeDataVector<geometry::Color> color_data;
				color_data.reserve(colors->count);
				for (int i = 0; i < colors->count; i++)
				{
					color_data.emplace_back(static_cast<uint8_t>(reinterpret_cast<float*>(colors->buffer.get())[i * 3] * 255.0f),
											static_cast<uint8_t>(reinterpret_cast<float*>(colors->buffer.get())[i * 3 + 1] * 255.0f),
											static_cast<uint8_t>(reinterpret_cast<float*>(colors->buffer.get())[i * 3 + 2] * 255.0f));
				}
				auto color_attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<geometry::Color>>("color", geometry::AttributeSemantic::Color, color_data);
				auto color_attribute_data = std::make_unique<geometry::AttributeData<geometry::Color>>(std::move(color_data));
				pointcloud->addAttribute(std::move(color_attribute_data), std::move(color_attribute_metadata));
			}
		}

		addCustomAttributes(pointcloud.get(), custom_attributes);
		return pointcloud;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}

	return nullptr;
}

}