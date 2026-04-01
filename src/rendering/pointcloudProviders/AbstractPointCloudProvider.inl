#include "AbstractPointCloudProvider.h"

namespace sahara::rendering
{

template <typename T>
const geometry::TypedAttributeMetadata<T>* AbstractPointCloudProvider::typedAttributeMetadata(geometry::AttributeSemantic semantic)
{
	return dynamic_cast<const geometry::TypedAttributeMetadata<T>*>(m_available_attributes.at(semantic));
}

}