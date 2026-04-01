#pragma once

#include <cstddef>

namespace sahara::geometry
{

// We need this abstract class, as the AttributeData is templated, but should be stored within the point cloud without having to specify the template parameter
class AbstractAttributeData
{
public:
	virtual ~AbstractAttributeData() = default;
	virtual size_t size() const = 0;
	virtual size_t sizeInBytes() const = 0;
	virtual const void* rawData() const = 0;
};

}