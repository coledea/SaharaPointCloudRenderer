#include "RasterizerSpecifications.h"

#include "HQComputeRasterizer.h"
#include "OOCPointPrimitiveRasterizer.h"
#include "PointComputeRasterizer.h"
#include "PointPrimitiveRasterizer.h"
#include "multitemporal/MultiTemporalPointPrimitiveRasterizer.h"

#include <cassert>
#include <utility>

namespace sahara::rendering
{

const std::set<geometry::AttributeSpecification> POSITION_ATTRIBUTE = {
	{ geometry::AttributeType::Vector3D, geometry::AttributeSemantic::Position }
};

const std::map<RasterizerType, RasterizerSpecifications> RasterizerSpecifications::Specifications = {
	{ RasterizerType::PointPrimitiveRasterizer,
	  RasterizerSpecifications(RasterizerType::PointPrimitiveRasterizer, POSITION_ATTRIBUTE, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) -> std::unique_ptr<AbstractRasterizer> {
		  return std::make_unique<PointPrimitiveRasterizer>(opengl_context, pointcloud_provider, camera);
	  }) },
	{ RasterizerType::PointComputeRasterizer, RasterizerSpecifications(RasterizerType::PointComputeRasterizer, POSITION_ATTRIBUTE, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) -> std::unique_ptr<AbstractRasterizer> {
		  return std::make_unique<PointComputeRasterizer>(opengl_context, pointcloud_provider, camera);
	  }) },
	{ RasterizerType::HQComputeRasterizer, RasterizerSpecifications(RasterizerType::HQComputeRasterizer, POSITION_ATTRIBUTE, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) -> std::unique_ptr<AbstractRasterizer> {
		  return std::make_unique<HQComputeRasterizer>(opengl_context, pointcloud_provider, camera);
	  }) },
	{ RasterizerType::MultiTemporalPointPrimitiveRasterizer, RasterizerSpecifications(RasterizerType::MultiTemporalPointPrimitiveRasterizer, POSITION_ATTRIBUTE, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) -> std::unique_ptr<AbstractRasterizer> {
		  return std::make_unique<MultiTemporalPointPrimitiveRasterizer>(opengl_context, pointcloud_provider, camera);
	  }) },
	{ RasterizerType::OOCPointPrimitiveRasterizer, RasterizerSpecifications(RasterizerType::OOCPointPrimitiveRasterizer, POSITION_ATTRIBUTE, [](OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) -> std::unique_ptr<AbstractRasterizer> {
		  return std::make_unique<OOCPointPrimitiveRasterizer>(opengl_context, pointcloud_provider, camera);
	  }) }
};

RasterizerSpecifications::RasterizerSpecifications(RasterizerType type, std::set<geometry::AttributeSpecification> necessary_attributes, FactoryMethod factory_method) noexcept
	: m_type(type)
	, m_necessary_attributes(std::move(necessary_attributes))
	, m_factory_method(factory_method)
{
}

RasterizerType RasterizerSpecifications::type() const noexcept
{
	return m_type;
}

const std::set<geometry::AttributeSpecification>& RasterizerSpecifications::necessaryAttributes() const noexcept
{
	return m_necessary_attributes;
}

std::unique_ptr<AbstractRasterizer> RasterizerSpecifications::createRasterizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) const
{
	assert(m_factory_method != nullptr);
	return m_factory_method(opengl_context, pointcloud_provider, camera);
}

}
