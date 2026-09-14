#pragma once

#include "geometry/AttributeSpecification.h"
#include "rendering/RendererModuleTypes.h"

#include <map>
#include <memory>
#include <set>

namespace sahara::navigation
{
class Camera;
}

namespace sahara::rendering
{

class AbstractPointCloudProvider;
class AbstractRasterizer;
class OpenGLContext;

class RasterizerSpecifications
{
public:
	using FactoryMethod = std::unique_ptr<AbstractRasterizer> (*)(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera);

	RasterizerSpecifications(RasterizerType type, std::set<geometry::AttributeSpecification> necessary_attributes, FactoryMethod factory_method) noexcept;

	RasterizerType type() const noexcept;
	const std::set<geometry::AttributeSpecification>& necessaryAttributes() const noexcept;
	std::unique_ptr<AbstractRasterizer> createRasterizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) const;

	static const std::map<RasterizerType, RasterizerSpecifications> Specifications;

private:
	RasterizerType m_type;
	std::set<geometry::AttributeSpecification> m_necessary_attributes;
	FactoryMethod m_factory_method;
};

}
