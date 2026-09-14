#include "ColorizerSpecifications.h"

#include "AttributeBasedColorizer.h"
#include "SingleColorColorizer.h"

namespace sahara::rendering
{

const std::map<ColorizerType, ColorizerSpecifications> ColorizerSpecifications::Specifications = {
	{ ColorizerType::SingleColor,
	  ColorizerSpecifications(ColorizerType::SingleColor, "Single Color", {}, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera*) -> std::unique_ptr<AbstractColorizer> {
		  return std::make_unique<SingleColorColorizer>(opengl_context, pointcloud_provider);
	  }) },
	{ ColorizerType::AttributeBased, ColorizerSpecifications(ColorizerType::AttributeBased, "Attribute-based", {}, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) -> std::unique_ptr<AbstractColorizer> {
		  return std::make_unique<AttributeBasedColorizer>(opengl_context, pointcloud_provider, camera);
	  }) }
};

ColorizerSpecifications::ColorizerSpecifications(ColorizerType type, QString name, std::set<geometry::AttributeSpecification> necessary_attributes, FactoryMethod factory_method) noexcept
	: m_type(type)
	, m_name(std::move(name))
	, m_necessary_attributes(std::move(necessary_attributes))
	, m_factory_method(factory_method)
{
}

ColorizerType ColorizerSpecifications::type() const noexcept
{
	return m_type;
}

const QString& ColorizerSpecifications::name() const noexcept
{
	return m_name;
}

const std::set<geometry::AttributeSpecification>& ColorizerSpecifications::necessaryAttributes() const noexcept
{
	return m_necessary_attributes;
}

std::unique_ptr<AbstractColorizer> ColorizerSpecifications::createColorizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) const
{
	assert(m_factory_method != nullptr);
	return m_factory_method(opengl_context, pointcloud_provider, camera);
}

}
