#include "PLYLoader.h"

namespace sahara::io
{

template <typename T_original, typename T_attribute>
void PLYLoader::addPointCloudAttribute(geometry::StaticPointCloud* pointcloud, std::shared_ptr<tinyply::PlyData> data, geometry::AttributeSemantic attribute_semantic, const QString& name)
{
	geometry::AttributeDataVector<T_attribute> raw_data;
	raw_data.assign(reinterpret_cast<const T_original*>(data->buffer.get()), reinterpret_cast<const T_original*>(data->buffer.get()) + data->count);
	auto attribute_metadata = std::make_unique<geometry::TypedAttributeMetadata<T_attribute>>(name, attribute_semantic, raw_data);
	auto attribute_data = std::make_unique<geometry::AttributeData<T_attribute>>(std::move(raw_data));
	pointcloud->addAttribute(std::move(attribute_data), std::move(attribute_metadata));
}

}