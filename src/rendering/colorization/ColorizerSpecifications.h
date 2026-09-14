#pragma once

#include "AbstractColorizer.h"
#include "geometry/AttributeSpecification.h"
#include "navigation/Camera.h"
#include "rendering/RendererModuleTypes.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

#include <map>
#include <set>

namespace sahara::rendering
{

class ColorizerSpecifications
{
public:
	using FactoryMethod = std::unique_ptr<AbstractColorizer> (*)(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera);

	ColorizerSpecifications(ColorizerType type, QString name, std::set<geometry::AttributeSpecification> necessary_attributes, FactoryMethod factory_method) noexcept;

	ColorizerType type() const noexcept;
	const QString& name() const noexcept;
	const std::set<geometry::AttributeSpecification>& necessaryAttributes() const noexcept;
	std::unique_ptr<AbstractColorizer> createColorizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) const;

	static const std::map<ColorizerType, ColorizerSpecifications> Specifications;

private:
	ColorizerType m_type;
	QString m_name;
	std::set<geometry::AttributeSpecification> m_necessary_attributes;
	FactoryMethod m_factory_method;
};

}
